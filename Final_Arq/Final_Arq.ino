/**
 * @file Final_Arq.ino
 * @brief Programa principal del Sistema de Confort Térmico.
 * @details Integra todos los módulos (sensores, actuadores, FSM, alarmas)
 *          y utiliza la librería AsyncTask para ejecutar tareas no bloqueantes.
 * @author [Tu Nombre]
 * @date 2025
 * @version 1.0
 */

#include "configuracion.h"
#include "sistema_confort.h"
#include "fsm.h"
#include "alarmas.h"
#include "AsyncTaskLib.h"

// Objeto global del sistema
SistemaConfort confort;

// ==================== DECLARACIÓN DE CALLBACKS ====================
void callbackLeerSensores();
void callbackActualizarLCD();
void callbackLeerTeclado();
void callbackControlAlarmas();

// ==================== CREACIÓN DE TAREAS ASINCRÓNICAS ====================
/**
 * @brief Tarea para lectura de sensores cada 2 segundos.
 */
AsyncTask tareaSensores(2000, true, callbackLeerSensores);

/**
 * @brief Tarea para actualizar LCD cada 500 ms.
 */
AsyncTask tareaLCD(500, true, callbackActualizarLCD);

/**
 * @brief Tarea para leer teclado cada 100 ms.
 */
AsyncTask tareaTeclado(100, true, callbackLeerTeclado);

/**
 * @brief Tarea para controlar alarmas cada 1 segundo.
 */
AsyncTask tareaAlarmas(1000, true, callbackControlAlarmas);

// ==================== IMPLEMENTACIÓN DE CALLBACKS ====================
void callbackLeerSensores() {
  confort.leerSensores();
}

void callbackActualizarLCD() {
  confort.actualizarLCD();
}

void callbackLeerTeclado() {
  confort.leerTeclado();
}

void callbackControlAlarmas() {
  confort.controlarAlarmas();
}

// ==================== SETUP ====================
void setup() {
  confort.begin();                // Inicializa periféricos
  confort.testHardware();         // Prueba rápida de todos los componentes
  setupFSM();                     // Inicializa máquina de estados
  inicializarAlarmas();           // Inicializa sistema de alarmas

  // Iniciar las tareas asincrónicas
  tareaSensores.Start();
  tareaLCD.Start();
  tareaTeclado.Start();
  tareaAlarmas.Start();

  Serial.println(F("Sistema iniciado. Tareas asincrónicas corriendo."));
}

// ==================== LOOP ====================
void loop() {
  // Actualizar todas las tareas (no bloqueante)
  tareaSensores.Update();
  tareaLCD.Update();
  tareaTeclado.Update();
  tareaAlarmas.Update();

  // Actualizar la máquina de estados
  loopFSM();
}