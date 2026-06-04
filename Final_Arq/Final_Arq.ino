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
#include <EEPROM.h>
#include <MFRC522.h>

// Objeto global del sistema
SistemaConfort confort;
AsyncTask tareaSensores(2000, true, callbackLeerSensores);
bool botonPresionado = false;
unsigned long tiempoUltimoBoton = 0;
String inputBuffer = "";
bool bufferCompleto = false;

void limpiarBuffer() { inputBuffer = ""; bufferCompleto = false; }
String obtenerBufferEntrada() { return inputBuffer; }

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
//AsyncTask tareaAlarmas(1000, true, callbackControlAlarmas);

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
 confort.begin();
  // confort.testHardware();   // Comentado para evitar delays
  ptrSistema = &confort;
  setupFSM();
  inicializarAlarmas();

  tareaSensores.Start();
  tareaLCD.Start();
  tareaTeclado.Start();
  // tareaAlarmas.Start();     // Comentado
  
  Serial.println(F("Sistema iniciado. Tareas asincrónicas corriendo."));
}

// ==================== LOOP ====================
void loop() {
  tareaSensores.Update();
  tareaLCD.Update();
  tareaTeclado.Update();
  // tareaAlarmas.Update();
  loopFSM();            // La FSM maneja todo
  
  // Detección de condiciones de alarma según estado actual
  static unsigned long lastCheck = 0;
  if (millis() - lastCheck > 500) {
    lastCheck = millis();
    // Estado 4: detectar sonido alto (umbral)
    if (getEstadoActual() == ESTADO_MONITOR_INTRUSOS) {
      if (confort.getSonidoAnalog() > SONIDO_UMBRAL) {
        dispararEvento(EVENTO_SONIDO_ALTO);
      }
    }
    // Estado 5: detectar condición ambiental
    else if (getEstadoActual() == ESTADO_MONITOR_AMBIENTAL) {
      if (confort.getTemperatura() < 20.0 && confort.getLuz() < 100) {
        dispararEvento(EVENTO_CONDICION_ALARMA_AMBIENTAL);
      }
    }
    // Estado Inicio: verificar RFID o clave ingresada (se maneja con buffer)
    else if (getEstadoActual() == ESTADO_INICIO) {
      // Comprobar si se ha ingresado una clave completa (ej. 4 dígitos)
      if (bufferCompleto) {
        if (confort.validarClave(buffer)) {
          dispararEvento(EVENTO_CLAVE_CORRECTA);
        } else {
          confort.incrementarIntentosFallidos();
          dispararEvento(EVENTO_CLAVE_INCORRECTA);
        }
        limpiarBuffer();
      }
      // También comprobar RFID
      if (confort.leerRFID()) {
        dispararEvento(EVENTO_CLAVE_CORRECTA);
      }
    }
  }
}