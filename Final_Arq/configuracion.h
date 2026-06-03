/**
 * @file configuracion.h
 * @brief Definición de pines, constantes y variables globales del sistema.
 * @details Este archivo centraliza la configuración de hardware (pines,
 *          umbrales de alarma, parámetros del termistor) y declara las
 *          variables globales compartidas entre módulos.
 */

#ifndef CONFIGURACION_H
#define CONFIGURACION_H

#include <Arduino.h>

// ==================== PINES PARA ARDUINO MEGA ====================

// Sensores
#define PIN_TERMISTOR A2      ///< Pin analógico para termistor NTC (temperatura)
#define PIN_LDR A3            ///< Pin analógico para fotoresistencia (luz)
#define PIN_HALL A1           ///< Pin analógico para sensor magnético Hall
#define PIN_SONIDO_DIGITAL 7  ///< Pin digital para sensor KY-037 (salida digital)
#define PIN_SONIDO_ANALOG A0  ///< Pin analógico para sensor KY-037 (intensidad de sonido)

// Actuadores
#define PIN_SERVO 13          ///< Pin para servomotor (persiana/ventilador)
#define PIN_BUZZER 6          ///< Pin para buzzer (cambiado a 6 para evitar conflicto)
#define PIN_LED_ALARMA 3      ///< Pin para LED rojo de alarma simple
#define PIN_LED_RGB_R 5       ///< Pin para canal rojo del LED RGB
#define PIN_LED_RGB_G 6       ///< Pin para canal verde del LED RGB (comparte con buzzer, ajustar si es necesario)
#define PIN_LED_RGB_B 8       ///< Pin para canal azul del LED RGB

// Teclado matricial 4x4
#define FILAS_KEYPAD 4        ///< Número de filas del teclado
#define COLUMNAS_KEYPAD 4     ///< Número de columnas del teclado
extern const byte filasKeypad[FILAS_KEYPAD];     ///< Pines de fila (definidos en sistema_confort.cpp)
extern const byte columnasKeypad[COLUMNAS_KEYPAD]; ///< Pines de columna
extern const char teclas[FILAS_KEYPAD][COLUMNAS_KEYPAD]; ///< Mapa de caracteres

// RFID RC522 (SPI)
#define PIN_RFID_SS 53        ///< Pin Slave Select para RC522
#define PIN_RFID_RST 9        ///< Pin Reset para RC522

// ==================== PARÁMETROS DEL TERMISTOR ====================
extern const float R1;        ///< Resistencia fija en serie (10k ohm)
extern const float c1, c2, c3; ///< Coeficientes de Steinhart-Hart

// ==================== UMBRALES DE ALARMA ====================
extern const float TEMP_MAX;  ///< Temperatura máxima permitida (°C)
extern const float TEMP_MIN;  ///< Temperatura mínima permitida (°C)
extern const int SONIDO_UMBRAL; ///< Umbral analógico para sensor de sonido

// ==================== VARIABLES GLOBALES ====================
extern float temperatura;     ///< Última temperatura leída (°C)
extern int luz;               ///< Último valor de luz (0-1023)
extern int campoMagnetico;    ///< Último valor del sensor Hall (0-1023)
extern int sonidoAnalog;      ///< Último valor analógico del sensor de sonido
extern bool sonidoDigital;    ///< Último valor digital del sensor de sonido
extern int contadorAlarmas;   ///< Número de alarmas consecutivas en ventana de 12s
extern unsigned long tiempoPrimeraAlarma; ///< Momento (ms) de la primera alarma de la ventana
extern bool emergenciaActiva; ///< Bandera que indica estado de emergencia crítica
extern char ultimaTecla;      ///< Última tecla presionada en el teclado

#endif // CONFIGURACION_H