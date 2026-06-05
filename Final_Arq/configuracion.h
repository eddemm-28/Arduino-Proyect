/**
 * @file configuracion.h
 * @brief Definición de pines, constantes y variables globales del sistema.
 */
 
#ifndef CONFIGURACION_H
#define CONFIGURACION_H
 
#include <Arduino.h>
 
// ==================== PINES PARA ARDUINO MEGA ====================
#define PIN_TERMISTOR A2
#define PIN_LDR A3
#define PIN_HALL A1
#define PIN_SONIDO_DIGITAL 8
#define PIN_SONIDO_ANALOG A0
 
#define PIN_SERVO 13
#define PIN_BUZZER 7
#define PIN_LED_ALARMA 24
#define PIN_LED_RGB_R 6
#define PIN_LED_RGB_G 25
#define PIN_LED_RGB_B 26
 
#define PIN_LCD_RS 12
#define PIN_LCD_EN 11
#define PIN_LCD_D4 5
#define PIN_LCD_D5 4
#define PIN_LCD_D6 3
#define PIN_LCD_D7 2
 
#define FILAS_KEYPAD 4
#define COLUMNAS_KEYPAD 4
extern const byte filasKeypad[FILAS_KEYPAD];
extern const byte columnasKeypad[COLUMNAS_KEYPAD];
extern const char teclas[FILAS_KEYPAD][COLUMNAS_KEYPAD];
 
#define PIN_RFID_SS 53
#define PIN_RFID_RST 9
 
// ==================== PARÁMETROS DEL TERMISTOR ====================
extern const float R1;
extern const float c1, c2, c3;
 
// ==================== UMBRALES DE ALARMA ====================
extern const float TEMP_MAX;
extern const float TEMP_MIN;
extern const int SONIDO_UMBRAL;
 
// ==================== VARIABLES GLOBALES ====================
extern float temperatura;
extern int luz;
extern int campoMagnetico;
extern int sonidoAnalog;
extern bool sonidoDigital;
extern int contadorAlarmas;
extern unsigned long tiempoPrimeraAlarma;
extern bool emergenciaActiva;
extern char ultimaTecla;
 
// ==================== BOTÓN EXTERNO ====================
#define PIN_BOTON 10
 
// ==================== EEPROM ====================
// Mapa de memoria:
//   [0]      → flag usuario válido (0x01 si hay credenciales)
//   [1..4]   → clave (4 chars ASCII, ej: "1234")
//   [5..12]  → UID RFID (8 chars ASCII, ej: "532E7C2E")
#define EEPROM_USUARIO_VALIDO  0
#define EEPROM_CLAVE_LENGTH    4
#define EEPROM_CLAVE_START     1
#define EEPROM_UID_LENGTH      4   // 4 bytes físicos del UID
#define EEPROM_UID_START       5   // ocupa posiciones 5..12 (8 chars)
 
// ==================== VARIABLES GLOBALES ADICIONALES ====================
extern bool botonPresionado;
extern unsigned long tiempoUltimoBoton;
 
#endif // CONFIGURACION_H