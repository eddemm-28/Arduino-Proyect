#include "fsm.h"
#include "configuracion.h"
#include "sistema_confort.h"
#include "AsyncTaskLib.h"
#include <EEPROM.h>

// ==== Declaración de funciones auxiliares (prototipos) ====
void detenerTemporizadores();
void incrementarContadorAlarmasGlobal();
void actualizarLEDyBuzzer();
void actualizarLCDporEstado();
extern String obtenerBufferEntrada();
extern String obtenerUIDLeido();
extern String inputBuffer;
extern bool bufferCompleto;
extern void limpiarBuffer();

// ==== Variables estáticas ====
static EstadoSistema estadoActual = ESTADO_INGRESAR_HORA;  // <-- CAMBIADO
static unsigned long tiempoUltimoEvento = 0;
static int contadorSonidoAlto = 0;
static unsigned long tiempoPrimerSonido = 0;
static int alarmasGlobales = 0;
static unsigned long inicioVentanaAlarmas = 0;
static bool enAlarmaPorIntruso = false;

static SubEstadoConfig subEstadoConfig = CONFIG_MENU;
static String nuevaClave = "";
static bool esperandoConfirmacion = false;
static unsigned long tiempoEsperaRFID = 0;
static String confirmacion = "";

// ==== Variables para hora y roles ====
// *** NUEVO: hora ingresada por el usuario y gestión de roles ***
static String horaBuffer = "";          // acumula los 4 dígitos HHMM
static int horaActual = -1;             // hora validada, ej: 1430
int rolActivo = -1;                     // rol validado (-1 = ninguno)

// Franjas horarias en RAM: [rol][0]=inicio, [rol][1]=fin
static int franjas[NUM_ROLES][4] = {
  {0,    559},   // Rol 0: 00:00 - 05:59
  {600,  1159},  // Rol 1: 06:00 - 11:59
  {1200, 1759},  // Rol 2: 12:00 - 17:59
  {1800, 2359}   // Rol 3: 18:00 - 23:59
};

// Variables para edición de franjas en configuración
static int rolEditando = -1;            // qué rol se está editando
static String horaFranjaBuffer = "";    // buffer para ingreso de hora inicio/fin
static bool editandoInicio = true;      // true=editando inicio, false=editando fin

// ==== Prototipos internos nuevos ====
void cargarFranjasDesdeEEPROM();
void guardarFranjasEnEEPROM();
int determinarRolPorHora(int hora);
bool horaEnFranja(int hora, int inicio, int fin);

// ==== Tareas asincrónicas ====
AsyncTask timer2s(2000, false, []() { dispararEvento(EVENTO_TIMER_2S); });
AsyncTask timer5s(5000, false, []() { dispararEvento(EVENTO_TIMER_5S); });
AsyncTask timerAlarma2s(2000, false, []() { dispararEvento(EVENTO_TIMER_2S_DESDE_ALARMA); });
AsyncTask timerAlarma4s(4000, false, []() { dispararEvento(EVENTO_TIMER_4S_DESDE_ALARMA); });

// Variable global
SistemaConfort *ptrSistema = nullptr;

// ==================== HELPERS DE FRANJAS ====================
bool horaEnFranja(int hora, int inicio, int fin) {
  // Soporta franjas que cruzan la medianoche (ej: 2200-0559)
  if (inicio <= fin) {
    return (hora >= inicio && hora <= fin);
  } else {
    return (hora >= inicio || hora <= fin);
  }
}

int determinarRolPorHora(int hora) {
  for (int r = 0; r < NUM_ROLES; r++) {
    if (horaEnFranja(hora, franjas[r][0], franjas[r][1])) {
      return r;
    }
  }
  return -1;  // hora no cae en ninguna franja
}

void cargarFranjasDesdeEEPROM() {
  // Si los bytes están en 0xFF (EEPROM virgen) se usan los valores por defecto
  byte check = EEPROM.read(EEPROM_FRANJAS_START);
  if (check == 0xFF) return;  // primera vez: conservar defaults
  for (int r = 0; r < NUM_ROLES; r++) {
    int base = EEPROM_FRANJAS_START + r * 4;
    franjas[r][0] = (int)EEPROM.read(base)     << 8 | (int)EEPROM.read(base + 1);
    franjas[r][1] = (int)EEPROM.read(base + 2) << 8 | (int)EEPROM.read(base + 3);
    franjas[r][2] = (int)EEPROM.read(base + 4)     << 8 | (int)EEPROM.read(base + 5);
    franjas[r][3] = (int)EEPROM.read(base + 6) << 8 | (int)EEPROM.read(base + 9);
  }
}

void guardarFranjasEnEEPROM() {
  for (int r = 0; r < NUM_ROLES; r++) {
    int base = EEPROM_FRANJAS_START + r * 4;
    EEPROM.write(base,     (franjas[r][0] >> 8) & 0xFF);
    EEPROM.write(base + 1,  franjas[r][1]       & 0xFF);
    EEPROM.write(base + 2, (franjas[r][2] >> 8) & 0xFF);
    EEPROM.write(base + 3,  franjas[r][3]       & 0xFF);
  }
}

// ==================== SETUP FSM ====================
void setupFSM() {
  estadoActual = ESTADO_INGRESAR_HORA;  // <-- CAMBIADO
  cargarFranjasDesdeEEPROM();
  Serial.println(F("FSM inicializada en INGRESAR_HORA"));
}

// ==================== LOOP FSM ====================
void loopFSM() {
  if (ptrSistema == nullptr) return;

  ptrSistema->leerBoton();
  if (botonPresionado) {
    dispararEvento(EVENTO_BOTON_RESET);
    botonPresionado = false;
  }

  timer2s.Update();
  timer5s.Update();
  timerAlarma2s.Update();
  timerAlarma4s.Update();

  actualizarLEDyBuzzer();
  actualizarLCDporEstado();
}

// ==================== DISPARAR EVENTO ====================
void dispararEvento(int evento) {
  Serial.print("Evento: ");
  Serial.println(evento);

  switch (estadoActual) {

    // *** NUEVO ESTADO: pedir hora antes de todo ***
    case ESTADO_INGRESAR_HORA:
      if (evento >= (int)'0' && evento <= (int)'9') {
        char d = (char)evento;
        if (horaBuffer.length() < 4) {
          horaBuffer += d;
        }
      }
      else if (evento == EVENTO_TECLA_HASH) {
        // '#' confirma la hora ingresada
        if (horaBuffer.length() == 4) {
          int hh = horaBuffer.substring(0,2).toInt();
          int mm = horaBuffer.substring(2,4).toInt();
          if (hh >= 0 && hh <= 23 && mm >= 0 && mm <= 59) {
            horaActual = hh * 100 + mm;
            rolActivo = determinarRolPorHora(horaActual);
            Serial.print(F("Hora ingresada: "));
            Serial.print(horaActual);
            Serial.print(F(" -> Rol: "));
            Serial.println(rolActivo);
            horaBuffer = "";
            limpiarBuffer();
            estadoActual = ESTADO_INICIO;
          } else {
            // Hora inválida: reiniciar buffer
            Serial.println(F("Hora invalida. Reingrese."));
            horaBuffer = "";
          }
        } else {
          Serial.println(F("Ingrese 4 digitos HHMM"));
          horaBuffer = "";
        }
      }
      else if (evento == EVENTO_TECLA_ASTERISCO) {
        // '*' borra el último dígito
        if (horaBuffer.length() > 0) {
          horaBuffer = horaBuffer.substring(0, horaBuffer.length() - 1);
        }
      }
      break;

    case ESTADO_INICIO:
      if (evento == EVENTO_CLAVE_CORRECTA) {
        // Verificar que hay un rol activo para esta hora
        if (rolActivo == -1) {
          Serial.println(F("Clave OK pero hora fuera de franja. Acceso denegado."));
          // Mostrar brevemente en LCD (se gestiona en actualizarLCDporEstado)
          // Volver a pedir hora
          horaBuffer = "";
          estadoActual = ESTADO_INGRESAR_HORA;
        } else {
          Serial.print(F("Acceso concedido. Rol: "));
          Serial.println(rolActivo);
          estadoActual = ESTADO_CONFIGURACION;
          detenerTemporizadores();
          if (ptrSistema) ptrSistema->moverServo(90);
        }
      }
      else if (evento == EVENTO_CLAVE_INCORRECTA) {
        int fallos = ptrSistema->getIntentosFallidos();
        if (ptrSistema && fallos >= 3) {
          estadoActual = ESTADO_BLOQUEO;
          digitalWrite(PIN_LED_ALARMA, HIGH);
          tone(PIN_BUZZER, 2000, 50);
          detenerTemporizadores();
        }
      }
      break;

    case ESTADO_BLOQUEO:
      if (evento == EVENTO_BOTON_RESET) {
        // Al desbloquear, volver a pedir hora
        horaBuffer = "";
        horaActual = -1;
        rolActivo = -1;
        estadoActual = ESTADO_INGRESAR_HORA;  // <-- CAMBIADO
        if (ptrSistema) ptrSistema->resetIntentosFallidos();
        detenerTemporizadores();
        Serial.println(F("-> INGRESAR_HORA (reset bloqueo)"));
      }
      break;

    case ESTADO_CONFIGURACION:
      if (evento == EVENTO_BOTON_RESET) {
        // Al salir por reset, volver a pedir hora
        horaBuffer = "";
        horaActual = -1;
        rolActivo = -1;
        estadoActual = ESTADO_INGRESAR_HORA;  // <-- CAMBIADO
        subEstadoConfig = CONFIG_MENU;
        nuevaClave = "";
        confirmacion = "";
        horaFranjaBuffer = "";
        rolEditando = -1;
        detenerTemporizadores();
        if (ptrSistema) ptrSistema->cargarCredencialesDesdeEEPROM();
        if (ptrSistema) ptrSistema->moverServo(0);
        Serial.println(F("-> INGRESAR_HORA (reset config)"));
      }
      else if (evento == EVENTO_RFID_DETECTADO && subEstadoConfig == CONFIG_REGISTRO_RFID) {
        String nuevoUID = obtenerUIDLeido();
        if (ptrSistema) {
          ptrSistema->guardarCredenciales(ptrSistema->getClaveAlmacenada(), nuevoUID);
          Serial.print(F("Nueva tarjeta registrada: "));
          Serial.println(nuevoUID);
        }
        subEstadoConfig = CONFIG_MENU;
      }
      else if (evento == EVENTO_TECLA_A) {
        if (subEstadoConfig == CONFIG_MENU) {
          estadoActual = ESTADO_MONITOR_INTRUSOS;
          subEstadoConfig = CONFIG_MENU;
          nuevaClave = "";
          confirmacion = "";
          horaFranjaBuffer = "";
          rolEditando = -1;
          detenerTemporizadores();
          if (ptrSistema) ptrSistema->cargarCredencialesDesdeEEPROM();
          timer2s.Start();
          Serial.println(F("-> MONITOR INTRUSOS"));
        } else {
          // Cancelar sub-estado activo
          subEstadoConfig = CONFIG_MENU;
          nuevaClave = "";
          confirmacion = "";
          horaFranjaBuffer = "";
          rolEditando = -1;
          Serial.println(F("Cancelado. Volviendo al menu."));
        }
      }
      else if (evento == EVENTO_TECLA_HASH) {
        if (subEstadoConfig == CONFIG_FRANJAS_HORA_INICIO) {
          // Confirmar hora de inicio de la franja
          if (horaFranjaBuffer.length() == 4) {
            int hh = horaFranjaBuffer.substring(0,2).toInt();
            int mm = horaFranjaBuffer.substring(2,4).toInt();
            if (hh >= 0 && hh <= 23 && mm >= 0 && mm <= 59) {
              franjas[rolEditando][0] = hh * 100 + mm;
              horaFranjaBuffer = "";
              editandoInicio = false;
              subEstadoConfig = CONFIG_FRANJAS_HORA_FIN;
              Serial.print(F("Inicio guardado: "));
              Serial.println(franjas[rolEditando][0]);
            } else {
              horaFranjaBuffer = "";
              Serial.println(F("Hora invalida."));
            }
          } else {
            horaFranjaBuffer = "";
          }
        }
        else if (subEstadoConfig == CONFIG_FRANJAS_HORA_FIN) {
          // Confirmar hora de fin de la franja
          if (horaFranjaBuffer.length() == 4) {
            int hh = horaFranjaBuffer.substring(0,2).toInt();
            int mm = horaFranjaBuffer.substring(2,4).toInt();
            if (hh >= 0 && hh <= 23 && mm >= 0 && mm <= 59) {
              franjas[rolEditando][1] = hh * 100 + mm;
              guardarFranjasEnEEPROM();
              horaFranjaBuffer = "";
              rolEditando = -1;
              subEstadoConfig = CONFIG_MENU;
              Serial.println(F("Franja guardada en EEPROM."));
            } else {
              horaFranjaBuffer = "";
              Serial.println(F("Hora invalida."));
            }
          } else {
            horaFranjaBuffer = "";
          }
        }
        else if (subEstadoConfig != CONFIG_MENU) {
          subEstadoConfig = CONFIG_MENU;
          nuevaClave = "";
          confirmacion = "";
          horaFranjaBuffer = "";
          rolEditando = -1;
          Serial.println(F("Cancelado. Volviendo al menu."));
        }
      }
      else if (evento == EVENTO_TECLA_ASTERISCO) {
        // '*' borra último dígito en sub-estados de franja
        if ((subEstadoConfig == CONFIG_FRANJAS_HORA_INICIO ||
             subEstadoConfig == CONFIG_FRANJAS_HORA_FIN) &&
            horaFranjaBuffer.length() > 0) {
          horaFranjaBuffer = horaFranjaBuffer.substring(0, horaFranjaBuffer.length() - 1);
        }
      }
      else {
        char tecla = (char)evento;

        switch (subEstadoConfig) {
          case CONFIG_MENU:
            if (tecla == '1') {
              subEstadoConfig = CONFIG_CAMBIO_CLAVE;
              nuevaClave = "";
              confirmacion = "";
              Serial.println(F("Modo: Cambiar clave"));
            }
            else if (tecla == '2') {
              subEstadoConfig = CONFIG_REGISTRO_RFID;
              tiempoEsperaRFID = millis();
              Serial.println(F("Modo: Registrar RFID"));
            }
            // *** NUEVO: opción 3 para editar franjas horarias ***
            else if (tecla == '3') {
              subEstadoConfig = CONFIG_FRANJAS_MENU;
              Serial.println(F("Modo: Franjas horarias"));
            }
            break;

          case CONFIG_CAMBIO_CLAVE:
            if (tecla >= '0' && tecla <= '9') {
              if (nuevaClave.length() < 4) {
                nuevaClave += tecla;
                if (nuevaClave.length() == 4) {
                  subEstadoConfig = CONFIG_CONFIRMAR_CLAVE;
                  Serial.println(F("Confirme la nueva clave"));
                }
              }
            }
            break;

          case CONFIG_CONFIRMAR_CLAVE:
            if (tecla >= '0' && tecla <= '9') {
              if (confirmacion.length() < 4) {
                confirmacion += tecla;
                if (confirmacion.length() == 4) {
                  if (confirmacion == nuevaClave) {
                    if (ptrSistema) {
                      ptrSistema->guardarCredenciales(nuevaClave, ptrSistema->getUIDAlmacenado());
                      Serial.println(F("Clave cambiada exitosamente"));
                    }
                  } else {
                    Serial.println(F("Error: las claves no coinciden"));
                  }
                  confirmacion = "";
                  nuevaClave = "";
                  subEstadoConfig = CONFIG_MENU;
                }
              }
            }
            break;

          case CONFIG_REGISTRO_RFID:
            if (millis() - tiempoEsperaRFID > 10000) {
              Serial.println(F("Tiempo agotado. Volviendo al menu."));
              subEstadoConfig = CONFIG_MENU;
            }
            break;

          // *** NUEVO: seleccionar qué rol editar ***
          case CONFIG_FRANJAS_MENU:
            if (tecla >= '1' && tecla <= '4') {
              rolEditando = tecla - '1';  // '1'->0, '2'->1, '3'->2, '4'->3
              horaFranjaBuffer = "";
              editandoInicio = true;
              subEstadoConfig = CONFIG_FRANJAS_HORA_INICIO;
              Serial.print(F("Editando Rol "));
              Serial.print(rolEditando + 1);
              Serial.print(F(" - Inicio actual: "));
              Serial.println(franjas[rolEditando][0]);
            }
            break;

          // Acumular dígitos para hora inicio/fin
          case CONFIG_FRANJAS_HORA_INICIO:
          case CONFIG_FRANJAS_HORA_FIN:
            if (tecla >= '0' && tecla <= '9') {
              if (horaFranjaBuffer.length() < 4) {
                horaFranjaBuffer += tecla;
              }
            }
            break;
        }
      }
      break;

    case ESTADO_MONITOR_INTRUSOS:
      if (evento == EVENTO_TIMER_2S) {
        estadoActual = ESTADO_MONITOR_AMBIENTAL;
        timer2s.Stop();
        timer5s.Start();
        Serial.println(F("-> MONITOR AMBIENTAL"));
      }
      else if (evento == EVENTO_TECLA_HASH) {
        estadoActual = ESTADO_CONFIGURACION;
        detenerTemporizadores();
        if (ptrSistema) ptrSistema->moverServo(90);
        Serial.println(F("-> CONFIGURACION (#)"));
      }
      else if (evento == EVENTO_SONIDO_ALTO || evento == EVENTO_HALL_DETECTADO) {
        if (contadorSonidoAlto == 0) tiempoPrimerSonido = millis();
        contadorSonidoAlto++;
        Serial.print(F("Deteccion intruso #"));
        Serial.println(contadorSonidoAlto);
        if (contadorSonidoAlto >= 3 && (millis() - tiempoPrimerSonido <= 12000)) {
          enAlarmaPorIntruso = true;
          estadoActual = ESTADO_ALARMA;
          detenerTemporizadores();
          timerAlarma2s.Start();
          incrementarContadorAlarmasGlobal();
          contadorSonidoAlto = 0;
          Serial.println(F("-> ALARMA (3 detecciones)"));
        }
        else if (millis() - tiempoPrimerSonido > 12000) {
          contadorSonidoAlto = 1;
          tiempoPrimerSonido = millis();
        }
      }
      break;

    case ESTADO_MONITOR_AMBIENTAL:
      if (evento == EVENTO_TIMER_5S) {
        estadoActual = ESTADO_MONITOR_INTRUSOS;
        timer5s.Stop();
        timer2s.Start();
        Serial.println(F("-> MONITOR INTRUSOS"));
      }
      else if (evento == EVENTO_TECLA_ASTERISCO) {
        estadoActual = ESTADO_CONFIGURACION;
        detenerTemporizadores();
        if (ptrSistema) ptrSistema->moverServo(90);
        Serial.println(F("-> CONFIGURACION (*)"));
      }
      else if (evento == EVENTO_CONDICION_ALARMA_AMBIENTAL) {
        enAlarmaPorIntruso = false;
        estadoActual = ESTADO_ALARMA;
        detenerTemporizadores();
        timerAlarma4s.Start();
        incrementarContadorAlarmasGlobal();
        Serial.println(F("-> ALARMA (temp/luz)"));
      }
      break;

    case ESTADO_ALARMA:
      if (evento == EVENTO_TIMER_2S_DESDE_ALARMA && enAlarmaPorIntruso) {
        estadoActual = ESTADO_MONITOR_INTRUSOS;
        detenerTemporizadores();
        timer2s.Start();
        Serial.println(F("-> MONITOR INTRUSOS (fin alarma)"));
      }
      else if (evento == EVENTO_TIMER_4S_DESDE_ALARMA && !enAlarmaPorIntruso) {
        estadoActual = ESTADO_MONITOR_AMBIENTAL;
        detenerTemporizadores();
        timer5s.Start();
        Serial.println(F("-> MONITOR AMBIENTAL (fin alarma)"));
      }
      else if (evento == EVENTO_TRES_ALARMAS_EN_12S) {
        horaBuffer = "";
        horaActual = -1;
        rolActivo = -1;
        estadoActual = ESTADO_INGRESAR_HORA;  // <-- CAMBIADO
        detenerTemporizadores();
        if (ptrSistema) ptrSistema->resetIntentosFallidos();
        if (ptrSistema) ptrSistema->moverServo(0);
        alarmasGlobales = 0;
        Serial.println(F("-> INGRESAR_HORA (3 alarmas en 12s)"));
      }
      break;
  }
}

// ==== Implementación de auxiliares ====
void detenerTemporizadores() {
  timer2s.Stop();
  timer5s.Stop();
  timerAlarma2s.Stop();
  timerAlarma4s.Stop();
}

void incrementarContadorAlarmasGlobal() {
  unsigned long ahora = millis();
  if (alarmasGlobales == 0) {
    inicioVentanaAlarmas = ahora;
    alarmasGlobales = 1;
  } else {
    if (ahora - inicioVentanaAlarmas <= 12000) {
      alarmasGlobales++;
      if (alarmasGlobales >= 3) {
        dispararEvento(EVENTO_TRES_ALARMAS_EN_12S);
        alarmasGlobales = 0;
      }
    } else {
      inicioVentanaAlarmas = ahora;
      alarmasGlobales = 1;
    }
  }
}

void actualizarLEDyBuzzer() {
  static unsigned long lastBlink = 0;
  static bool ledState = false;

  switch (estadoActual) {
    case ESTADO_INGRESAR_HORA:  // *** NUEVO: mismo LED que INICIO (apagado) ***
      digitalWrite(PIN_LED_ALARMA, LOW);
      noTone(PIN_BUZZER);
      analogWrite(PIN_LED_RGB_R, 0);
      analogWrite(PIN_LED_RGB_G, 0);
      analogWrite(PIN_LED_RGB_B, 0);
      break;

    case ESTADO_BLOQUEO:
    {
      static unsigned long lastChange = 0;
      static bool ledOn = true;
      if (ledOn) {
        if (millis() - lastChange >= 100) {
          digitalWrite(PIN_LED_ALARMA, LOW);
          ledOn = false;
          lastChange = millis();
        } else {
          digitalWrite(PIN_LED_ALARMA, HIGH);
        }
      } else {
        if (millis() - lastChange >= 500) {
          digitalWrite(PIN_LED_ALARMA, HIGH);
          ledOn = true;
          lastChange = millis();
          tone(PIN_BUZZER, 2000, 50);
        } else {
          digitalWrite(PIN_LED_ALARMA, LOW);
        }
      }
      digitalWrite(PIN_LED_RGB_R, LOW);
    }
    break;

    case ESTADO_ALARMA:
    {
      static bool ledAlarmaOn = true;
      static unsigned long tCambioAlarma = 0;
      unsigned long ahora = millis();
      if (ledAlarmaOn) {
        digitalWrite(PIN_LED_ALARMA, HIGH);
        if (ahora - tCambioAlarma >= 300) {
          ledAlarmaOn = false;
          tCambioAlarma = ahora;
        }
      } else {
        digitalWrite(PIN_LED_ALARMA, LOW);
        if (ahora - tCambioAlarma >= 700) {
          ledAlarmaOn = true;
          tCambioAlarma = ahora;
        }
      }
      tone(PIN_BUZZER, 2500);
      digitalWrite(PIN_LED_RGB_R, HIGH);
    }
    break;

    case ESTADO_CONFIGURACION:
      digitalWrite(PIN_LED_ALARMA, LOW);
      noTone(PIN_BUZZER);
      analogWrite(PIN_LED_RGB_R, 0);
      analogWrite(PIN_LED_RGB_G, 0);
      analogWrite(PIN_LED_RGB_B, 255);
      break;

    case ESTADO_MONITOR_INTRUSOS:
      digitalWrite(PIN_LED_ALARMA, LOW);
      noTone(PIN_BUZZER);
      analogWrite(PIN_LED_RGB_R, 0);
      analogWrite(PIN_LED_RGB_G, 255);
      analogWrite(PIN_LED_RGB_B, 0);
      break;

    case ESTADO_MONITOR_AMBIENTAL:
      digitalWrite(PIN_LED_ALARMA, LOW);
      noTone(PIN_BUZZER);
      analogWrite(PIN_LED_RGB_R, 255);
      analogWrite(PIN_LED_RGB_G, 0);
      analogWrite(PIN_LED_RGB_B, 0);
      break;

    default: // INICIO
      digitalWrite(PIN_LED_ALARMA, LOW);
      noTone(PIN_BUZZER);
      analogWrite(PIN_LED_RGB_R, 0);
      analogWrite(PIN_LED_RGB_G, 0);
      analogWrite(PIN_LED_RGB_B, 0);
      break;
  }
}

void actualizarLCDporEstado() {
  if (!ptrSistema) return;
  static unsigned long ultimaActualizacion = 0;
  if (millis() - ultimaActualizacion < 300) return;
  ultimaActualizacion = millis();

  LiquidCrystal &lcd = ptrSistema->getLCD();
  lcd.clear();

  switch (estadoActual) {

    // *** NUEVO: pantalla para ingresar hora ***
    case ESTADO_INGRESAR_HORA:
      lcd.setCursor(0,0);
      lcd.print("Hora (HHMM):#OK");
      lcd.setCursor(0,1);
      lcd.print(horaBuffer);
      for (int i = horaBuffer.length(); i < 4; i++) lcd.print("_");
      lcd.print(" *:borrar");
      break;

    case ESTADO_INICIO:
      lcd.setCursor(0,0);
      if (rolActivo == -1) {
        lcd.print("Sin acceso hora ");
      } else {
        lcd.print("Rol ");
        lcd.print(rolActivo + 1);
        lcd.print(" - Ingrese ID:");
      }
      lcd.setCursor(0,1);
      lcd.print(obtenerBufferEntrada());
      break;

    case ESTADO_BLOQUEO:
      lcd.setCursor(0,0);
      lcd.print("SISTEMA BLOQUEADO");
      lcd.setCursor(0,1);
      lcd.print("Presione boton");
      break;

    case ESTADO_MONITOR_INTRUSOS:
      lcd.setCursor(0,0);
      lcd.print("Intrusos H:");
      lcd.print(ptrSistema->getCampoMagnetico());
      lcd.setCursor(0,1);
      lcd.print("Snd:");
      lcd.print(ptrSistema->getSonidoAnalog());
      lcd.print(" #:Config");
      break;

    case ESTADO_MONITOR_AMBIENTAL:
      lcd.setCursor(0,0);
      lcd.print("Temp:");
      lcd.print(ptrSistema->getTemperatura(),1);
      lcd.print("C Luz:");
      lcd.print(map(ptrSistema->getLuz(),0,1023,0,100));
      lcd.print("%");
      lcd.setCursor(0,1);
      lcd.print("*:Config");
      break;

    case ESTADO_ALARMA:
      lcd.setCursor(0,0);
      lcd.print("!!! ALARMA !!!");
      lcd.setCursor(0,1);
      lcd.print(enAlarmaPorIntruso ? "Intrusos" : "Ambiental");
      break;

    case ESTADO_CONFIGURACION:
      lcd.setCursor(0,0);
      switch (subEstadoConfig) {
        case CONFIG_MENU:
          lcd.print("1:Clv 2:RFID    ");
          lcd.setCursor(0,1);
          lcd.print("3:Franjas  A:Sal");
          break;
        case CONFIG_CAMBIO_CLAVE:
          lcd.print("Nueva clave:    ");
          lcd.setCursor(0,1);
          lcd.print(nuevaClave);
          for (int i = nuevaClave.length(); i < 4; i++) lcd.print("_");
          break;
        case CONFIG_CONFIRMAR_CLAVE:
          lcd.print("Confirme:       ");
          lcd.setCursor(0,1);
          lcd.print(confirmacion);
          for (int i = confirmacion.length(); i < 4; i++) lcd.print("_");
          break;
        case CONFIG_REGISTRO_RFID:
          lcd.print("Pase tarjeta... ");
          lcd.setCursor(0,1);
          lcd.print("#:Cancel");
          break;
        // *** NUEVO: pantallas para edición de franjas ***
        case CONFIG_FRANJAS_MENU:
          lcd.print("Rol? 1-2-3-4    ");
          lcd.setCursor(0,1);
          lcd.print("A:cancelar      ");
          break;
        case CONFIG_FRANJAS_HORA_INICIO:
          lcd.print("R");
          lcd.print(rolEditando + 1);
          lcd.print(" Inicio(HHMM):#");
          lcd.setCursor(0,1);
          lcd.print(horaFranjaBuffer);
          for (int i = horaFranjaBuffer.length(); i < 4; i++) lcd.print("_");
          lcd.print(" *:borrar");
          break;
        case CONFIG_FRANJAS_HORA_FIN:
          lcd.print("R");
          lcd.print(rolEditando + 1);
          lcd.print(" Fin   (HHMM):#");
          lcd.setCursor(0,1);
          lcd.print(horaFranjaBuffer);
          for (int i = horaFranjaBuffer.length(); i < 4; i++) lcd.print("_");
          lcd.print(" *:borrar");
          break;
      }
      break;
  }
}

EstadoSistema getEstadoActual() {
  return estadoActual;
}

SubEstadoConfig getSubEstadoConfig() {
  return subEstadoConfig;
}