/**
 * @file sistema_confort.cpp
 * @brief Implementación de la clase SistemaConfort.
 * @details Contiene toda la lógica de lectura de sensores, control de actuadores,
 *          gestión de alarmas y prueba de hardware.
 */

#include "sistema_confort.h"
#include "configuracion.h"
#include "fsm.h"      // <-- AGREGADO para getEstadoActual()

extern String inputBuffer;
extern bool bufferCompleto;

// ... (el resto del archivo permanece igual, excepto la corrección del comentario en el constructor)
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
const float R1 = 10000.0;              // Resistencia fija en serie (10k ohm)
const float c1 = 0.001129148;          // Coeficiente Steinhart-Hart
const float c2 = 0.000234125;          // Coeficiente Steinhart-Hart
const float c3 = 0.0000000876741;      // Coeficiente Steinhart-Hart

// ==================== UMBRALES DE ALARMA ====================
const float TEMP_MAX = 35.0;           // Temperatura máxima antes de alarma (°C)
const float TEMP_MIN = 10.0;           // Temperatura mínima antes de alarma (°C)
const int SONIDO_UMBRAL = 800;         // Valor analógico a partir del cual se considera ruido fuerte

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
  : lcd(PIN_LCD_RS, PIN_LCD_EN, PIN_LCD_D4, PIN_LCD_D5, PIN_LCD_D6, PIN_LCD_D7),  // <- Cambio
    teclado(makeKeymap(teclas), filasKeypad, columnasKeypad, FILAS_KEYPAD, COLUMNAS_KEYPAD),
    rfid(PIN_RFID_SS, PIN_RFID_RST)
  {
  // Inicializar variables (los objetos ya se construyeron)
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
  // delay eliminado
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
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("T:");
  lcd.print(temperatura, 1);
  lcd.print("C L:");
  lcd.print(map(luz, 0, 1023, 0, 100));
  lcd.print("%");
  
  lcd.setCursor(0, 1);
  if (emergenciaActiva) {
    lcd.print("EMERGENCIA!   ");
  } else if (contadorAlarmas > 0) {
    lcd.print("ALARMA #");
    lcd.print(contadorAlarmas);
    lcd.print("    ");
  } else {
    lcd.print("Sistema OK     ");
  }
}

void SistemaConfort::leerTeclado() {
  char tecla = teclado.getKey();
  if (tecla) {
    ultimaTecla = tecla;
    procesarTecla(tecla);
  }
}

void SistemaConfort::controlarAlarmas() {
  bool condicionPeligro = (temperatura > TEMP_MAX || temperatura < TEMP_MIN) ||
                          (sonidoAnalog > SONIDO_UMBRAL);
  
  static bool alarmaAnterior = false;
  static unsigned long inicioVentana = 0;
  
  if (condicionPeligro && !alarmaAnterior) {
    // Nueva alarma detectada
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
      myservo.write(0);               // Detener servo
      digitalWrite(PIN_LED_ALARMA, HIGH);
      digitalWrite(PIN_LED_RGB_R, HIGH);
      tone(PIN_BUZZER, 2500);         // Tono continuo de emergencia
      Serial.println("!!! EMERGENCIA: 3 ALARMAS EN 12 SEGUNDOS !!!");
    } else {
      // Alarma simple: parpadeo de LED de alarma y pitido corto
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
Serial.print("Tecla presionada: ");
  Serial.println(tecla);
  
  // Si el estado actual es INICIO, acumular dígitos en el buffer
  if (getEstadoActual() == ESTADO_INICIO) {
    if (tecla >= '0' && tecla <= '9') {
      if (inputBuffer.length() < 4) {
        inputBuffer += tecla;
        if (inputBuffer.length() == 4) {
          bufferCompleto = true;
        }
      }
    } else if (tecla == '#') {
      // Forzar verificación si se presiona # antes de 4 dígitos
      bufferCompleto = true;
    }
  }
}
// Carga credenciales desde EEPROM a las variables cache
void SistemaConfort::cargarCredencialesDesdeEEPROM() {
  byte valido = EEPROM.read(EEPROM_USUARIO_VALIDO);
  if (valido == 0x01) {
    // Leer clave (4 dígitos)
    char buf[EEPROM_CLAVE_LENGTH+1];
    for (int i = 0; i < EEPROM_CLAVE_LENGTH; i++) {
      buf[i] = EEPROM.read(EEPROM_CLAVE_START + i);
    }
    buf[EEPROM_CLAVE_LENGTH] = '\0';
    claveAlmacenada = String(buf);
    
    // Leer UID (4 bytes en hex)
    String uid = "";
    for (int i = 0; i < EEPROM_UID_LENGTH; i++) {
      byte b = EEPROM.read(EEPROM_UID_START + i);
      if (b < 0x10) uid += "0";
      uid += String(b, HEX);
    }
    uidAlmacenado = uid;
  } else {
    // Valores por defecto para pruebas (clave 1234, UID simulado)
    claveAlmacenada = "1234";
    uidAlmacenado = "DEADBEEF";
    guardarCredenciales(claveAlmacenada, uidAlmacenado);
  }
}

void SistemaConfort::guardarCredenciales(String clave, String uid) {
  EEPROM.write(EEPROM_USUARIO_VALIDO, 0x01);
  for (int i = 0; i < EEPROM_CLAVE_LENGTH && i < clave.length(); i++) {
    EEPROM.write(EEPROM_CLAVE_START + i, clave[i]);
  }
  // Convertir UID string a bytes (asume formato hexadecimal, ej. "DEADBEEF")
  int len = uid.length();
  for (int i = 0; i < EEPROM_UID_LENGTH; i++) {
    if (i*2+1 < len) {
      String byteStr = uid.substring(i*2, i*2+2);
      byte b = (byte) strtol(byteStr.c_str(), NULL, 16);
      EEPROM.write(EEPROM_UID_START + i, b);
    } else {
      EEPROM.write(EEPROM_UID_START + i, 0);
    }
  }
  EEPROM.commit(); // Solo para placas que lo requieran (Mega no necesita, pero seguro)
  cargarCredencialesDesdeEEPROM();
}

bool SistemaConfort::validarClave(String entrada) {
  return (entrada == claveAlmacenada);
}

bool SistemaConfort::validarUID(String uid) {
  return (uid == uidAlmacenado);
}

void SistemaConfort::leerBoton() {
  bool lectura = digitalRead(PIN_BOTON);
  if (lectura == LOW && millis() - tiempoUltimoBoton > 200) { // debounce
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
  
  if (validarUID(uid)) {
    resetIntentosFallidos();
    return true;
  }
  incrementarIntentosFallidos();
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