#ifndef CONFIGURACION_H
#define CONFIGURACION_H

// Pines ajustados según los ejemplos reales
#define PIN_DHT 22
#define PIN_LDR A3          // Era A0 → ahora A3 (ejemplo LDR)
#define PIN_HALL A1         // Antes PIN_PRESION, ahora sensor Hall (ejemplo A1)
#define PIN_MOTOR 13        // Servo (ejemplo en pin 13)
#define PIN_LED_ALARMA 3    // LED rojo independiente para alarmas (como en proyecto original)
#define PIN_LED_RGB_R 5     // RGB rojo
#define PIN_LED_RGB_G 6     // RGB verde
#define PIN_LED_RGB_B 8     // RGB azul (se cambió de 7 a 8 para no chocar con buzzer)
#define PIN_BUZZER 7        // Buzzer (ejemplo pin 7)
#define PIN_RFID_SS 53
#define PIN_RFID_RST 9      // Antes 48 → ahora 9 (ejemplo RFID)

// Teclado matricial 4x4 (pines ajustados según ejemplo)
#define FILAS_KEYPAD 4
#define COLUMNAS_KEYPAD 4
extern const byte filasKeypad[FILAS_KEYPAD];
extern const byte columnasKeypad[COLUMNAS_KEYPAD];
extern const char teclas[FILAS_KEYPAD][COLUMNAS_KEYPAD];

// Variables globales (extern)
extern float temperatura;
extern float humedad;
extern int luz;
extern int hall;            // antes "presion"
extern int contadorAlarmas;
extern unsigned long tiempoPrimeraAlarma;

#endif