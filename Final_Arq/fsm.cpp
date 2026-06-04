#include "fsm.h"
#include "configuracion.h"
#include "sistema_confort.h"
#include "AsyncTaskLib.h"

// ==== Declaración de funciones auxiliares (prototipos) ====
void detenerTemporizadores();
void incrementarContadorAlarmasGlobal();
void actualizarLEDyBuzzer();
void actualizarLCDporEstado();
extern String obtenerBufferEntrada();
extern String obtenerUIDLeido();

// ==== Variables estáticas ====
static EstadoSistema estadoActual = ESTADO_INICIO;
static unsigned long tiempoUltimoEvento = 0;
static int contadorSonidoAlto = 0;
static unsigned long tiempoPrimerSonido = 0;
static int alarmasGlobales = 0;
static unsigned long inicioVentanaAlarmas = 0;
static bool enAlarmaPorIntruso = false;

// Al inicio, después de las otras variables estáticas:
static SubEstadoConfig subEstadoConfig = CONFIG_MENU;
static String nuevaClave = "";
static bool esperandoConfirmacion = false;
static unsigned long tiempoEsperaRFID = 0;  // timeout para lectura RFID

static String confirmacion = "";

// ==== Tareas asincrónicas ====
AsyncTask timer2s(2000, false, []() { dispararEvento(EVENTO_TIMER_2S); });
AsyncTask timer5s(5000, false, []() { dispararEvento(EVENTO_TIMER_5S); });
AsyncTask timerAlarma2s(2000, false, []() { dispararEvento(EVENTO_TIMER_2S_DESDE_ALARMA); });
AsyncTask timerAlarma4s(4000, false, []() { dispararEvento(EVENTO_TIMER_4S_DESDE_ALARMA); });

// Variable global
SistemaConfort *ptrSistema = nullptr;

void setupFSM() {
  estadoActual = ESTADO_INICIO;
  Serial.println(F("FSM inicializada en INICIO"));
  ptrSistema = nullptr;
}

void loopFSM() {
  if (ptrSistema == nullptr) return;
  
  ptrSistema->leerBoton();
  if (botonPresionado) {
    dispararEvento(EVENTO_BOTON_RESET);
    botonPresionado = false;
  }
  
  char tecla = ultimaTecla;
  if (tecla != 0) {
    ultimaTecla = 0;
    if (tecla == '#') dispararEvento(EVENTO_TECLA_HASH);
    else if (tecla == '*') dispararEvento(EVENTO_TECLA_ASTERISCO);
    else if (tecla == 'A' || tecla == 'a') dispararEvento(EVENTO_TECLA_A);
  }
  
  timer2s.Update();
  timer5s.Update();
  timerAlarma2s.Update();
  timerAlarma4s.Update();
  
  actualizarLEDyBuzzer();
  actualizarLCDporEstado();
}

void dispararEvento(int evento) {
  Serial.print("Evento: ");
  Serial.println(evento);
  
  switch (estadoActual) {
    case ESTADO_INICIO:
      if (evento == EVENTO_CLAVE_CORRECTA) {
        estadoActual = ESTADO_CONFIGURACION;
        Serial.println("-> CONFIGURACION");
        detenerTemporizadores();
      } 
      else if (evento == EVENTO_CLAVE_INCORRECTA) {
        if (ptrSistema && ptrSistema->getIntentosFallidos() >= 3) {
          estadoActual = ESTADO_BLOQUEO;
          Serial.println("-> BLOQUEO");
          detenerTemporizadores();
        }
      }
      break;
      
    case ESTADO_BLOQUEO:
      if (evento == EVENTO_BOTON_RESET) {
        estadoActual = ESTADO_INICIO;
        Serial.println("-> INICIO");
        if (ptrSistema) ptrSistema->resetIntentosFallidos();
        detenerTemporizadores();
      }
      break;
      
    case ESTADO_CONFIGURACION:
  if (evento == EVENTO_BOTON_RESET) {
    estadoActual = ESTADO_INICIO;
    subEstadoConfig = CONFIG_MENU;
    nuevaClave = "";
    Serial.println("-> INICIO (reset)");
    detenerTemporizadores();
  }
  else if (evento == EVENTO_TECLA_A) {
    // Salir de configuración y pasar a monitor intrusos
    estadoActual = ESTADO_MONITOR_INTRUSOS;
    subEstadoConfig = CONFIG_MENU;
    nuevaClave = "";
    Serial.println("-> MONITOR INTRUSOS");
    detenerTemporizadores();
    timer2s.Start();
  }
  else {
    // Manejo de sub-estados
    char tecla = (char)evento;  // Los eventos de tecla se pasan como su valor ASCII
    switch (subEstadoConfig) {
      case CONFIG_MENU:
        if (tecla == '1') {
          subEstadoConfig = CONFIG_CAMBIO_CLAVE;
          nuevaClave = "";
          Serial.println("Modo: Cambiar clave - Ingrese 4 digitos");
        }
        else if (tecla == '2') {
          subEstadoConfig = CONFIG_REGISTRO_RFID;
          tiempoEsperaRFID = millis();
          Serial.println("Modo: Registrar RFID - Pase la tarjeta");
        }
        break;
        
      case CONFIG_CAMBIO_CLAVE:
        if (tecla >= '0' && tecla <= '9') {
          if (nuevaClave.length() < 4) {
            nuevaClave += tecla;
            if (nuevaClave.length() == 4) {
              subEstadoConfig = CONFIG_CONFIRMAR_CLAVE;
              Serial.println("Confirme la nueva clave (mismos 4 digitos)");
            }
          }
        }
        else if (tecla == '#') {
          // Cancelar
          subEstadoConfig = CONFIG_MENU;
          nuevaClave = "";
          Serial.println("Cancelado. Volviendo al menu.");
        }
        break;
        
      case CONFIG_CONFIRMAR_CLAVE:
        if (tecla >= '0' && tecla <= '9') {
          static String confirmacion = "";
          if (confirmacion.length() < 4) {
            confirmacion += tecla;
            if (confirmacion.length() == 4) {
              if (confirmacion == nuevaClave) {
                if (ptrSistema) {
                  ptrSistema->guardarCredenciales(nuevaClave, ptrSistema->getUIDAlmacenado());
                  Serial.println("Clave cambiada exitosamente");
                }
              } else {
                Serial.println("Error: las claves no coinciden");
              }
              confirmacion = "";
              subEstadoConfig = CONFIG_MENU;
              nuevaClave = "";
            }
          }
        }
        else if (tecla == '#') {
          // Cancelar
          confirmacion = "";
          subEstadoConfig = CONFIG_MENU;
          nuevaClave = "";
          Serial.println("Cancelado.");
        }
        break;
        
      case CONFIG_REGISTRO_RFID:
        if (evento == EVENTO_RFID_DETECTADO) {
          String nuevoUID = obtenerUIDLeido();   // Ahora existe
          if (ptrSistema) {
            ptrSistema->guardarCredenciales(ptrSistema->getClaveAlmacenada(), nuevoUID);
            Serial.print("Nueva tarjeta registrada: ");
            Serial.println(nuevoUID);
          }
          subEstadoConfig = CONFIG_MENU;
        }
        if (millis() - tiempoEsperaRFID > 10000) {
          Serial.println("Tiempo de espera agotado. Volviendo al menu.");
          subEstadoConfig = CONFIG_MENU;
        }
        break;
    }
  }
  break;
      
    case ESTADO_MONITOR_INTRUSOS:
      if (evento == EVENTO_TIMER_2S) {
        estadoActual = ESTADO_MONITOR_AMBIENTAL;
        Serial.println("-> MONITOR AMBIENTAL");
        timer2s.Stop();
        timer5s.Start();
      }
      else if (evento == EVENTO_TECLA_HASH) {
        estadoActual = ESTADO_CONFIGURACION;
        Serial.println("-> CONFIGURACION (#)");
        detenerTemporizadores();
      }
      else if (evento == EVENTO_SONIDO_ALTO) {
        if (contadorSonidoAlto == 0) tiempoPrimerSonido = millis();
        contadorSonidoAlto++;
        if (contadorSonidoAlto >= 3 && (millis() - tiempoPrimerSonido <= 12000)) {
          enAlarmaPorIntruso = true;
          estadoActual = ESTADO_ALARMA;
          Serial.println("-> ALARMA (3 sonidos)");
          detenerTemporizadores();
          timerAlarma2s.Start();
          incrementarContadorAlarmasGlobal();
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
        Serial.println("-> MONITOR INTRUSOS (timer)");
        timer5s.Stop();
        timer2s.Start();
      }
      else if (evento == EVENTO_TECLA_ASTERISCO) {
        estadoActual = ESTADO_CONFIGURACION;
        Serial.println("-> CONFIGURACION (*)");
        detenerTemporizadores();
      }
      else if (evento == EVENTO_CONDICION_ALARMA_AMBIENTAL) {
        enAlarmaPorIntruso = false;
        estadoActual = ESTADO_ALARMA;
        Serial.println("-> ALARMA (temp/luz)");
        detenerTemporizadores();
        timerAlarma4s.Start();
        incrementarContadorAlarmasGlobal();
      }
      break;
      
    case ESTADO_ALARMA:
      if (evento == EVENTO_TIMER_2S_DESDE_ALARMA && enAlarmaPorIntruso) {
        estadoActual = ESTADO_MONITOR_INTRUSOS;
        Serial.println("-> MONITOR INTRUSOS (fin alarma)");
        detenerTemporizadores();
        timer2s.Start();
      }
      else if (evento == EVENTO_TIMER_4S_DESDE_ALARMA && !enAlarmaPorIntruso) {
        estadoActual = ESTADO_MONITOR_AMBIENTAL;
        Serial.println("-> MONITOR AMBIENTAL (fin alarma)");
        detenerTemporizadores();
        timer5s.Start();
      }
      else if (evento == EVENTO_TRES_ALARMAS_EN_12S) {
        estadoActual = ESTADO_INICIO;
        Serial.println("-> INICIO (3 alarmas en 12s)");
        detenerTemporizadores();
        if (ptrSistema) ptrSistema->resetIntentosFallidos();
        alarmasGlobales = 0;
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
    case ESTADO_BLOQUEO:
      if (millis() - lastBlink > 100) {
        ledState = !ledState;
        digitalWrite(PIN_LED_ALARMA, ledState);
        if (ledState) {
          tone(PIN_BUZZER, 2000, 50);   // Pitido corto de 2kHz durante 50ms
        }
        lastBlink = millis();
      }
      digitalWrite(PIN_LED_RGB_R, LOW);
      break;
      
    case ESTADO_ALARMA:
      if (millis() - lastBlink > 300) {
        ledState = !ledState;
        digitalWrite(PIN_LED_ALARMA, ledState);
        lastBlink = millis();
      }
      tone(PIN_BUZZER, 2500);
      digitalWrite(PIN_LED_RGB_R, HIGH);
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
  LiquidCrystal &lcd = ptrSistema->getLCD();
  lcd.clear();
  
  switch (estadoActual) {
    case ESTADO_INICIO:
      lcd.setCursor(0,0);
      lcd.print("Ingrese ID:");
      lcd.setCursor(0,1);
      lcd.print(obtenerBufferEntrada());
      break;
      
    case ESTADO_BLOQUEO:
      lcd.setCursor(0,0);
      lcd.print("SISTEMA BLOQUEADO");
      lcd.setCursor(0,1);
      lcd.print("Presione boton");
      break;
      
    case ESTADO_CONFIGURACION:
      lcd.setCursor(0,0);
      lcd.print("CONFIGURACION   ");
      lcd.setCursor(0,1);
      switch (subEstadoConfig) {
        case CONFIG_MENU:
          lcd.print("1:Clave 2:RFID A:Salir");
          break;
        case CONFIG_CAMBIO_CLAVE:
          lcd.print("Nueva clave: ");
          lcd.print(nuevaClave);
          for (int i = nuevaClave.length(); i < 4; i++) lcd.print("_");
          break;
        case CONFIG_CONFIRMAR_CLAVE:
          lcd.print("Confirme: ");
          lcd.print(confirmacion);  // Necesitas una variable estática para confirmación
          for (int i = confirmacion.length(); i < 4; i++) lcd.print("_");
          break;
        case CONFIG_REGISTRO_RFID:
          lcd.print("Pase tarjeta...");
          break;
      }
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
  }
}

EstadoSistema getEstadoActual() {
  return estadoActual;
}

SubEstadoConfig getSubEstadoConfig() {
  return subEstadoConfig;
}