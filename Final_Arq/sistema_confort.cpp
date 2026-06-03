#include "sistema_confort.h"
#include <LiquidCrystal_I2C.h>
#include <Keypad.h>
#include <DHT.h>

// Objetos globales necesarios (se definen aquí y se usan en el .ino)
LiquidCrystal_I2C lcd(0x27, 16, 2);
DHT dht(PIN_DHT, DHT11);
// Keypad se inicializará con los pines definidos en configuracion.h

// Variables globales definidas (para que linker no falle)
float temperatura = 0;
float humedad = 0;
int luz = 0;
int presion = 0;
int contadorAlarmas = 0;
unsigned long tiempoPrimeraAlarma = 0;

const byte filasKeypad[FILAS_KEYPAD] = {26,27,28,29};
const byte columnasKeypad[COLUMNAS_KEYPAD] = {30,31,32,33};
const char teclas[FILAS_KEYPAD][COLUMNAS_KEYPAD] = {
  {'1','2','3','A'},
  {'4','5','6','B'},
  {'7','8','9','C'},
  {'*','0','#','D'}
};
Keypad teclado = Keypad(makeKeymap(teclas), filasKeypad, columnasKeypad, FILAS_KEYPAD, COLUMNAS_KEYPAD);

SistemaConfort::SistemaConfort() {
  tiempoUltimoSensor = 0;
  tiempoUltimoLCD = 0;
  tiempoUltimoTeclado = 0;
  tiempoUltimoAlarma = 0;
}

void SistemaConfort::begin() {
  Serial.begin(9600);
  lcd.init();
  lcd.backlight();
  dht.begin();
  pinMode(PIN_LDR, INPUT);
  pinMode(PIN_PRESION, INPUT);
  pinMode(PIN_MOTOR, OUTPUT);
  pinMode(PIN_LED_ROJO, OUTPUT);
  pinMode(PIN_LED_VERDE, OUTPUT);
  pinMode(PIN_LED_AZUL, OUTPUT);
  pinMode(PIN_BUZZER, OUTPUT);
  digitalWrite(PIN_MOTOR, LOW);
  digitalWrite(PIN_LED_ROJO, LOW);
  digitalWrite(PIN_LED_VERDE, LOW);
  digitalWrite(PIN_LED_AZUL, LOW);
  digitalWrite(PIN_BUZZER, LOW);
  lcd.print("Sistema listo");
  delay(2000);
  lcd.clear();
}

void SistemaConfort::tickSensores() {
  // Implementar lógica no bloqueante (usar millis)
}

void SistemaConfort::tickLCD() {}
void SistemaConfort::tickTeclado() {}
void SistemaConfort::tickAlarmas() {}

float SistemaConfort::getTemperatura() { return temperatura; }
float SistemaConfort::getHumedad() { return humedad; }
int SistemaConfort::getLuz() { return luz; }
int SistemaConfort::getPresion() { return presion; }

void SistemaConfort::leerDHT() { /* leer y actualizar variables */ }
void SistemaConfort::leerLDR() { luz = analogRead(PIN_LDR); }
void SistemaConfort::leerPresion() { presion = analogRead(PIN_PRESION); }
void SistemaConfort::actualizarLCD() {}
void SistemaConfort::procesarTecla(char tecla) {}
void SistemaConfort::evaluarAlarmas() {}