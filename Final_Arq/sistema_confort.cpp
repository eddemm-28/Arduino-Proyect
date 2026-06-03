/**
 * @file sistema_confort.cpp
 * @brief Implementación de la clase SistemaConfort.
 * @details Contiene toda la lógica de lectura de sensores, control de actuadores,
 *          gestión de alarmas y prueba de hardware.
 */

#include "sistema_confort.h"
#include "configuracion.h"

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
  : lcd(0x27, 16, 2),                             // Dirección I2C común 0x27, LCD 16x2
    teclado(makeKeymap(teclas), filasKeypad, columnasKeypad, FILAS_KEYPAD, COLUMNAS_KEYPAD) {
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
}

// ==================== MÉTODOS PÚBLICOS ====================
void SistemaConfort::begin() {
  Serial.begin(9600);
  lcd.init();
  lcd.backlight();
  lcd.print("Iniciando...");
  
  myservo.attach(PIN_SERVO);
  myservo.write(0);               // Posición inicial (cerrado)
  
  pinMode(PIN_LED_ALARMA, OUTPUT);
  pinMode(PIN_LED_RGB_R, OUTPUT);
  pinMode(PIN_LED_RGB_G, OUTPUT);
  pinMode(PIN_LED_RGB_B, OUTPUT);
  pinMode(PIN_BUZZER, OUTPUT);
  pinMode(PIN_SONIDO_DIGITAL, INPUT);
  
  // Apagar todos los actuadores
  digitalWrite(PIN_LED_ALARMA, LOW);
  digitalWrite(PIN_LED_RGB_R, LOW);
  digitalWrite(PIN_LED_RGB_G, LOW);
  digitalWrite(PIN_LED_RGB_B, LOW);
  digitalWrite(PIN_BUZZER, LOW);
  
  lcd.clear();
  lcd.print("Sistema listo");
  delay(2000);
  lcd.clear();
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
  Serial.println(F("\n=== INICIANDO TEST DE HARDWARE ==="));
  lcd.clear();
  lcd.print("TEST HARDWARE");
  delay(1500);
  
  // 1. Termistor
  lcd.clear();
  lcd.print("1. Termistor");
  leerTermistor();
  Serial.print(F("Temperatura: ")); Serial.print(temperatura); Serial.println(" C");
  lcd.setCursor(0,1);
  lcd.print("T:"); lcd.print(temperatura,1);
  delay(2000);
  
  // 2. LDR
  lcd.clear();
  lcd.print("2. LDR");
  leerLDR();
  Serial.print(F("Luz: ")); Serial.println(luz);
  lcd.setCursor(0,1);
  lcd.print("Luz:"); lcd.print(luz);
  delay(2000);
  
  // 3. Hall
  lcd.clear();
  lcd.print("3. Hall (iman)");
  leerHall();
  Serial.print(F("Hall: ")); Serial.println(campoMagnetico);
  lcd.setCursor(0,1);
  lcd.print("Hall:"); lcd.print(campoMagnetico);
  delay(2000);
  
  // 4. Teclado
  lcd.clear();
  lcd.print("4. Teclado");
  lcd.setCursor(0,1);
  lcd.print("Presione tecla");
  Serial.println(F("Esperando tecla..."));
  char tecla = 0;
  while (!tecla) {
    tecla = teclado.getKey();
  }
  Serial.print(F("Tecla: ")); Serial.println(tecla);
  lcd.clear();
  lcd.print("Tecla: "); lcd.print(tecla);
  delay(1000);
  
  // 5. Servo
  lcd.clear();
  lcd.print("5. Servo");
  lcd.setCursor(0,1);
  lcd.print("Moviendo...");
  Serial.println(F("Moviendo servo 0° -> 90° -> 0°"));
  myservo.write(0); delay(500);
  myservo.write(90); delay(1000);
  myservo.write(0); delay(500);
  
  // 6. LED alarma
  lcd.clear();
  lcd.print("6. LED alarma");
  Serial.println(F("Parpadeo LED alarma 3 veces"));
  for (int i=0; i<3; i++) {
    digitalWrite(PIN_LED_ALARMA, HIGH); delay(300);
    digitalWrite(PIN_LED_ALARMA, LOW);  delay(300);
  }
  
  // 7. Buzzer
  lcd.clear();
  lcd.print("7. Buzzer");
  Serial.println(F("Sonido buzzer 1s"));
  tone(PIN_BUZZER, 1000); delay(1000);
  noTone(PIN_BUZZER);
  
  // 8. LED RGB
  lcd.clear();
  lcd.print("8. RGB (R,G,B)");
  Serial.println(F("Secuencia rojo, verde, azul"));
  analogWrite(PIN_LED_RGB_R, 255); analogWrite(PIN_LED_RGB_G, 0); analogWrite(PIN_LED_RGB_B, 0); delay(1000);
  analogWrite(PIN_LED_RGB_R, 0);   analogWrite(PIN_LED_RGB_G, 255); analogWrite(PIN_LED_RGB_B, 0); delay(1000);
  analogWrite(PIN_LED_RGB_R, 0);   analogWrite(PIN_LED_RGB_G, 0);   analogWrite(PIN_LED_RGB_B, 255); delay(1000);
  analogWrite(PIN_LED_RGB_R, 0);   analogWrite(PIN_LED_RGB_G, 0);   analogWrite(PIN_LED_RGB_B, 0);
  
  lcd.clear();
  lcd.print("TEST COMPLETO");
  Serial.println(F("=== TEST COMPLETADO CON EXITO ==="));
  delay(2000);
  lcd.clear();
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
  // Aquí se conectará con la FSM (por ahora solo se imprime)
  Serial.print("Tecla presionada: ");
  Serial.println(tecla);
  // Ejemplo básico: control manual del servo con teclas 2,8,6
  // if (tecla == '2') myservo.write(0);
  // else if (tecla == '8') myservo.write(90);
  // else if (tecla == '6') myservo.write(180);
}