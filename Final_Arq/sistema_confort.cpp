/**
 * @file sistema_confort.cpp
 * @brief Implementación de la clase SistemaConfort.
 * @details Contiene toda la lógica de lectura de sensores, control de actuadores,
 *          gestión de alarmas y prueba de hardware.
 */
 
#include "sistema_confort.h"
#include "configuracion.h"
#include "fsm.h"
 
extern String inputBuffer;
extern bool bufferCompleto;
 
// ==================== DEFINICIÓN DE PINES DEL TECLADO ====================
const byte filasKeypad[FILAS_KEYPAD] = {32, 33, 34, 35};
const byte columnasKeypad[COLUMNAS_KEYPAD] = {A7, A6, A5, A4};
const char teclas[FILAS_KEYPAD][COLUMNAS_KEYPAD] = {
  {'1','2','3','A'},
  {'4','5','6','B'},
  {'7','8','9','C'},
  {'*','0','#','D'}
};
 
// ==================== PARÁMETROS DEL TERMISTOR ====================
const float R1 = 10000.0;
const float c1 = 0.001129148;
const float c2 = 0.000234125;
const float c3 = 0.0000000876741;
 
// ==================== UMBRALES DE ALARMA ====================
const float TEMP_MAX = 35.0;
const float TEMP_MIN = 10.0;
const int SONIDO_UMBRAL = 800;
 
// ==================== VARIABLES GLOBALES (definición) ====================
float temperatura = 0;
int luz = 0;
int campoMagnetico = 0;
int sonidoAnalog = 0;
bool sonidoDigital = false;
int contadorAlarmas = 0;
unsigned long tiempoPrimeraAlarma = 0;
bool emergenciaActiva = false;
char ultimaTecla = 0;
 
// ==================== CONSTRUCTOR ====================
SistemaConfort::SistemaConfort()
  : lcd(PIN_LCD_RS, PIN_LCD_EN, PIN_LCD_D4, PIN_LCD_D5, PIN_LCD_D6, PIN_LCD_D7),
    teclado(makeKeymap(teclas), filasKeypad, columnasKeypad, FILAS_KEYPAD, COLUMNAS_KEYPAD),
    rfid(PIN_RFID_SS, PIN_RFID_RST)
  {
  temperatura = 0;
  luz = 0;
  campoMagnetico = 0;
  sonidoAnalog = 0;
  sonidoDigital = false;
  alarmasConsecutivas = 0;
  tiempoPrimeraAlarma = 0;
  emergenciaActiva = false;
  ultimaTecla = 0;
  intentosFallidos = 0;
  claveAlmacenada = "";
  uidAlmacenado = "";
}
 
// ==================== MÉTODOS PÚBLICOS ====================
void SistemaConfort::begin() {
  Serial.begin(9600);
  lcd.begin(16, 2);
  lcd.print("Iniciando...");
  
  myservo.attach(PIN_SERVO);
  myservo.write(0);
  
  pinMode(PIN_LED_ALARMA, OUTPUT);
  pinMode(PIN_LED_RGB_R, OUTPUT);
  pinMode(PIN_LED_RGB_G, OUTPUT);
  pinMode(PIN_LED_RGB_B, OUTPUT);
  pinMode(PIN_BUZZER, OUTPUT);
  pinMode(PIN_SONIDO_DIGITAL, INPUT);
  
  digitalWrite(PIN_LED_ALARMA, LOW);
  digitalWrite(PIN_LED_RGB_R, LOW);
  digitalWrite(PIN_LED_RGB_G, LOW);
  digitalWrite(PIN_LED_RGB_B, LOW);
  digitalWrite(PIN_BUZZER, LOW);
  
  lcd.clear();
  lcd.print("Sistema listo");
  lcd.clear();
 
  SPI.begin();
  rfid.PCD_Init();
  
  pinMode(PIN_BOTON, INPUT_PULLUP);
  botonPresionado = false;
  tiempoUltimoBoton = 0;
 
  cargarCredencialesDesdeEEPROM();
}
 
void SistemaConfort::leerSensores() {
  leerTermistor();
  leerLDR();
  leerHall();
  leerSonido();
}
 
void SistemaConfort::actualizarLCD() {
  // Este método ya no controla el LCD principal; la FSM lo gestiona
  // mediante actualizarLCDporEstado(). Se mantiene por compatibilidad.
}
 
void SistemaConfort::leerTeclado() {
  char tecla = teclado.getKey();
  if (tecla) {
    Serial.print("Tecla presionada: ");
    Serial.println(tecla);
 
    // Acumular buffer solo en ESTADO_INICIO
    procesarTecla(tecla);
 
    // *** CORRECCIÓN DEFINITIVA: disparar evento aquí mismo, sin pasar
    // por ultimaTecla ni esperar a loopFSM(). Esto elimina cualquier
    // problema de timing entre tareaTeclado y loopFSM. ***
    if (tecla == '#') {
      dispararEvento(EVENTO_TECLA_HASH);
    } else if (tecla == '*') {
      dispararEvento(EVENTO_TECLA_ASTERISCO);
    } else if (tecla == 'A' || tecla == 'a') {
      dispararEvento(EVENTO_TECLA_A);
    } else if (tecla == 'B' || tecla == 'b' ||
               tecla == 'C' || tecla == 'c' ||
               tecla == 'D' || tecla == 'd') {
      // Teclas B/C/D no usadas actualmente, ignorar
    } else {
      // Teclas numéricas: enviar ASCII
      dispararEvento((int)tecla);
    }
  }
}
 
void SistemaConfort::controlarAlarmas() {
  bool condicionPeligro = (temperatura > TEMP_MAX || temperatura < TEMP_MIN) ||
                          (sonidoAnalog > SONIDO_UMBRAL);
  
  static bool alarmaAnterior = false;
  static unsigned long inicioVentana = 0;
  
  if (condicionPeligro && !alarmaAnterior) {
    if (alarmasConsecutivas == 0) {
      inicioVentana = millis();
      tiempoPrimeraAlarma = inicioVentana;
      alarmasConsecutivas = 1;
    } else {
      if (millis() - inicioVentana <= 12000) {
        alarmasConsecutivas++;
      } else {
        inicioVentana = millis();
        alarmasConsecutivas = 1;
      }
    }
    
    if (alarmasConsecutivas >= 3 && (millis() - inicioVentana <= 12000)) {
      emergenciaActiva = true;
      myservo.write(0);
      digitalWrite(PIN_LED_ALARMA, HIGH);
      digitalWrite(PIN_LED_RGB_R, HIGH);
      tone(PIN_BUZZER, 2500);
      Serial.println("!!! EMERGENCIA: 3 ALARMAS EN 12 SEGUNDOS !!!");
    } else {
      static unsigned long ultimoParpadeo = 0;
      if (millis() - ultimoParpadeo > 500) {
        ultimoParpadeo = millis();
        digitalWrite(PIN_LED_ALARMA, !digitalRead(PIN_LED_ALARMA));
        digitalWrite(PIN_BUZZER, !digitalRead(PIN_BUZZER));
      }
    }
    alarmaAnterior = true;
  } else if (!condicionPeligro) {
    if (!emergenciaActiva) {
      digitalWrite(PIN_LED_ALARMA, LOW);
      digitalWrite(PIN_BUZZER, LOW);
      digitalWrite(PIN_LED_RGB_R, LOW);
    }
    if (alarmasConsecutivas > 0 && (millis() - inicioVentana > 12000)) {
      alarmasConsecutivas = 0;
    }
    alarmaAnterior = false;
  }
}
 
void SistemaConfort::testHardware() {
  Serial.println(F("Test hardware no implementado sin delays."));
}
 
// ==================== MÉTODOS PRIVADOS ====================
void SistemaConfort::leerTermistor() {
  int Vo = analogRead(PIN_TERMISTOR);
  float R2 = R1 * (1023.0 / (float)Vo - 1.0);
  float logR2 = log(R2);
  float Tk = 1.0 / (c1 + c2*logR2 + c3*logR2*logR2*logR2);
  temperatura = Tk - 273.15;
}
 
void SistemaConfort::leerLDR() {
  luz = analogRead(PIN_LDR);
}
 
void SistemaConfort::leerHall() {
  campoMagnetico = analogRead(PIN_HALL);
}
 
void SistemaConfort::leerSonido() {
  sonidoAnalog = analogRead(PIN_SONIDO_ANALOG);
  sonidoDigital = digitalRead(PIN_SONIDO_DIGITAL);
}
 
void SistemaConfort::procesarTecla(char tecla) {
  // *** CORRECCIÓN: procesarTecla SOLO acumula el buffer en ESTADO_INICIO.
  // Para todos los demás estados, loopFSM() lee ultimaTecla directamente
  // y dispara el evento correcto. Mezclar lógica aquí causaba que las
  // teclas en ESTADO_CONFIGURACION no produjeran ningún evento. ***
  if (getEstadoActual() == ESTADO_INICIO) {
    if (tecla >= '0' && tecla <= '9') {
      if (inputBuffer.length() < 4) {
        inputBuffer += tecla;
        if (inputBuffer.length() == 4) {
          bufferCompleto = true;
        }
      }
    } else if (tecla == '#') {
      bufferCompleto = true;
    }
  }
}
 
void SistemaConfort::cargarCredencialesDesdeEEPROM() {
  byte valido = EEPROM.read(EEPROM_USUARIO_VALIDO);
  if (valido == 0x01) {
    // Leer clave (4 caracteres)
    char bufClave[EEPROM_CLAVE_LENGTH + 1];
    for (int i = 0; i < EEPROM_CLAVE_LENGTH; i++) {
      bufClave[i] = (char)EEPROM.read(EEPROM_CLAVE_START + i);
    }
    bufClave[EEPROM_CLAVE_LENGTH] = '\0';
    claveAlmacenada = String(bufClave);
 
    // Leer UID: se guardan 8 chars ASCII (ej "532E7C2E") en EEPROM_UID_LENGTH*2 bytes
    char bufUID[EEPROM_UID_LENGTH * 2 + 1];
    for (int i = 0; i < EEPROM_UID_LENGTH * 2; i++) {
      bufUID[i] = (char)EEPROM.read(EEPROM_UID_START + i);
    }
    bufUID[EEPROM_UID_LENGTH * 2] = '\0';
    uidAlmacenado = String(bufUID);
    uidAlmacenado.toUpperCase();  // siempre en mayúsculas para comparar
 
    Serial.print("Credenciales cargadas - Clave: ");
    Serial.print(claveAlmacenada);
    Serial.print(" UID: ");
    Serial.println(uidAlmacenado);
  } else {
    // Primera vez: no hay RFID registrado aún, solo clave por defecto
    claveAlmacenada = "1234";
    uidAlmacenado = "";  // sin RFID registrado
    // Guardar clave por defecto pero dejar UID vacío
    EEPROM.write(EEPROM_USUARIO_VALIDO, 0x01);
    for (int i = 0; i < EEPROM_CLAVE_LENGTH; i++) {
      EEPROM.write(EEPROM_CLAVE_START + i, claveAlmacenada[i]);
    }
    for (int i = 0; i < EEPROM_UID_LENGTH; i++) {
      EEPROM.write(EEPROM_UID_START + i, 0);
    }
    Serial.println("Primera inicializacion: clave=1234, sin RFID registrado");
  }
}
 
void SistemaConfort::guardarCredenciales(String clave, String uid) {
  uid.toUpperCase();  // normalizar siempre a mayúsculas antes de guardar
 
  EEPROM.write(EEPROM_USUARIO_VALIDO, 0x01);
 
  // Guardar clave como caracteres ASCII
  for (int i = 0; i < EEPROM_CLAVE_LENGTH; i++) {
    if (i < (int)clave.length()) {
      EEPROM.write(EEPROM_CLAVE_START + i, (byte)clave[i]);
    } else {
      EEPROM.write(EEPROM_CLAVE_START + i, 0);
    }
  }
 
  // Guardar UID como string ASCII de 8 chars (ej: "532E7C2E")
  // Se usan EEPROM_UID_LENGTH*2 posiciones (8 bytes para 8 caracteres)
  for (int i = 0; i < EEPROM_UID_LENGTH * 2; i++) {
    if (i < (int)uid.length()) {
      EEPROM.write(EEPROM_UID_START + i, (byte)uid[i]);
    } else {
      EEPROM.write(EEPROM_UID_START + i, 0);
    }
  }
 
  // Actualizar cache en RAM inmediatamente (sin releer EEPROM)
  claveAlmacenada = clave;
  uidAlmacenado = uid;
 
  Serial.print(">>> Credenciales guardadas OK - Clave: ");
  Serial.print(claveAlmacenada);
  Serial.print(" | UID activo: ");
  Serial.println(uidAlmacenado);
}
 
bool SistemaConfort::validarClave(String entrada) {
  return (entrada == claveAlmacenada);
}
 
bool SistemaConfort::validarUID(String uid) {
  return (uid == uidAlmacenado);
}
 
void SistemaConfort::leerBoton() {
  bool lectura = digitalRead(PIN_BOTON);
  if (lectura == LOW && millis() - tiempoUltimoBoton > 200) {
    tiempoUltimoBoton = millis();
    botonPresionado = true;
  } else if (lectura == HIGH) {
    botonPresionado = false;
  }
}
 
bool SistemaConfort::leerRFID() {
  if (!rfid.PICC_IsNewCardPresent()) return false;
  if (!rfid.PICC_ReadCardSerial()) return false;
  
  String uid = "";
  for (byte i = 0; i < rfid.uid.size; i++) {
    if (rfid.uid.uidByte[i] < 0x10) uid += "0";
    uid += String(rfid.uid.uidByte[i], HEX);
  }
  uid.toUpperCase();
  rfid.PICC_HaltA();
  rfid.PCD_StopCrypto1();
  
  Serial.print("RFID leido: ");
  Serial.print(uid);
  Serial.print(" | UID esperado: ");
  Serial.println(uidAlmacenado);
 
  if (validarUID(uid)) {
    resetIntentosFallidos();
    Serial.println(">>> RFID OK: acceso concedido");
    return true;
  }
  // Tarjeta no reconocida: NO incrementa intentos fallidos.
  Serial.println(">>> RFID no reconocido");
  return false;
}
 
int SistemaConfort::getIntentosFallidos() {
  return intentosFallidos;
}
 
void SistemaConfort::incrementarIntentosFallidos() {
  intentosFallidos++;
}
 
void SistemaConfort::resetIntentosFallidos() {
  intentosFallidos = 0;
}
 
String SistemaConfort::leerCualquierRFID() {
  if (!rfid.PICC_IsNewCardPresent()) return "";
  if (!rfid.PICC_ReadCardSerial()) return "";
  String uid = "";
  for (byte i = 0; i < rfid.uid.size; i++) {
    if (rfid.uid.uidByte[i] < 0x10) uid += "0";
    uid += String(rfid.uid.uidByte[i], HEX);
  }
  uid.toUpperCase();
  rfid.PICC_HaltA();
  rfid.PCD_StopCrypto1();
  return uid;
}