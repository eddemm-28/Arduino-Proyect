#ifndef CONFIGURACION_H
#define CONFIGURACION_H

// Pines para Arduino Mega
#define PIN_DHT 22
#define PIN_LDR A0
#define PIN_PRESION A1
#define PIN_MOTOR 9
#define PIN_LED_ROJO 5
#define PIN_LED_VERDE 6
#define PIN_LED_AZUL 7
#define PIN_BUZZER 8
#define PIN_RFID_SS 53
#define PIN_RFID_RST 48

// Teclado matricial 4x4 (pines Mega)
#define FILAS_KEYPAD 4
#define COLUMNAS_KEYPAD 4
extern const byte filasKeypad[FILAS_KEYPAD];
extern const byte columnasKeypad[COLUMNAS_KEYPAD];
extern const char teclas[FILAS_KEYPAD][COLUMNAS_KEYPAD];

// Variables globales (extern)
extern float temperatura;
extern float humedad;
extern int luz;
extern int presion;
extern int contadorAlarmas;
extern unsigned long tiempoPrimeraAlarma;

#endif