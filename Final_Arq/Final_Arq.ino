/**
 * @file Final_Arq.ino
 * @brief Programa principal del Sistema de Confort Térmico.
 * @details Integra todos los módulos (sensores, actuadores, FSM, alarmas)
 *          y utiliza la librería AsyncTask para ejecutar tareas no bloqueantes.
 * @version 1.0
 */

#include "configuracion.h"
#include "sistema_confort.h"
#include "fsm.h"
#include "alarmas.h"
#include "AsyncTaskLib.h"
#include <EEPROM.h>
#include <MFRC522.h>

// ==================== DECLARACIÓN DE CALLBACKS (ANTES DE SU USO) ====================
void callbackLeerSensores();
void callbackActualizarLCD();
void callbackLeerTeclado();
void callbackControlAlarmas();

// ==================== OBJETOS GLOBALES ====================
SistemaConfort confort;
bool botonPresionado = false;
unsigned long tiempoUltimoBoton = 0;
String inputBuffer = "";
bool bufferCompleto = false;

void limpiarBuffer() { inputBuffer = ""; bufferCompleto = false; }
String obtenerBufferEntrada() { return inputBuffer; }

// ==================== CREACIÓN DE TAREAS ASINCRÓNICAS ====================
//AsyncTask tareaSensores(2000, true, callbackLeerSensores);
AsyncTask tareaLCD(500, true, callbackActualizarLCD);
AsyncTask tareaTeclado(100, true, callbackLeerTeclado);
// AsyncTask tareaAlarmas(1000, true, callbackControlAlarmas);  // No se usa

// ==================== IMPLEMENTACIÓN DE CALLBACKS ====================
void callbackLeerSensores() {
  confort.leerSensores();
}

void callbackActualizarLCD() {
  confort.actualizarLCD();   // Esta función ya no actualiza la FSM, pero se mantiene
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
  ptrSistema = &confort;
  setupFSM();
  inicializarAlarmas();

  tareaSensores.Start();
  tareaLCD.Start();
  tareaTeclado.Start();
  
  Serial.println(F("Sistema iniciado. Tareas asincrónicas corriendo."));
}

// ==================== LOOP ====================
void loop() {
  tareaSensores.Update();
  tareaLCD.Update();
  tareaTeclado.Update();
  loopFSM();
  
  static unsigned long lastCheck = 0;
  if (millis() - lastCheck > 500) {
    lastCheck = millis();
    
    if (getEstadoActual() == ESTADO_MONITOR_INTRUSOS) {
      if (confort.getSonidoAnalog() > SONIDO_UMBRAL) {
        dispararEvento(EVENTO_SONIDO_ALTO);
      }
    }
    else if (getEstadoActual() == ESTADO_MONITOR_AMBIENTAL) {
      if (confort.getTemperatura() < 20.0 && confort.getLuz() < 100) {
        dispararEvento(EVENTO_CONDICION_ALARMA_AMBIENTAL);
      }
    }
    else if (getEstadoActual() == ESTADO_INICIO) {
      if (bufferCompleto) {
        if (confort.validarClave(inputBuffer)) {   // CORREGIDO: buffer -> inputBuffer
          dispararEvento(EVENTO_CLAVE_CORRECTA);
        } else {
          confort.incrementarIntentosFallidos();
          dispararEvento(EVENTO_CLAVE_INCORRECTA);
        }
        limpiarBuffer();
      }
      if (confort.leerRFID()) {
        dispararEvento(EVENTO_CLAVE_CORRECTA);
      }
    }
  }
}