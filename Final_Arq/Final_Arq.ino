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
AsyncTask tareaSensores(2000, true, callbackLeerSensores);   // cada 2 segundos
AsyncTask tareaLCD(500, true, callbackActualizarLCD);        // cada 0.5 segundos
AsyncTask tareaTeclado(100, true, callbackLeerTeclado);      // cada 100 ms
AsyncTask tareaAlarmas(1000, true, callbackControlAlarmas);  // cada 1 segundo

// ==================== IMPLEMENTACIÓN DE CALLBACKS ====================
void callbackLeerSensores() {
  confort.leerSensores();   // Llama al método que lee DHT, LDR, presión
}

void callbackActualizarLCD() {
  confort.actualizarLCD();   // Actualiza la pantalla con los últimos datos
}

void callbackLeerTeclado() {
  confort.leerTeclado();     // Revisa teclas presionadas y las procesa
}

void callbackControlAlarmas() {
  confort.controlarAlarmas(); // Evalúa condiciones y maneja el contador de 3 alarmas en 12s
}

// ==================== SETUP ====================
void setup() {
  confort.begin();           // Inicializa pines, LCD, sensores, etc.
  setupFSM();                // Inicializa la máquina de estados
  inicializarAlarmas();      // Inicializa contadores de alarmas

  // Iniciar las tareas asincrónicas
  tareaSensores.Start();
  tareaLCD.Start();
  tareaTeclado.Start();
  tareaAlarmas.Start();

  Serial.println(F("Sistema iniciado. Tareas asincrónicas corriendo."));
}

// ==================== LOOP ====================
void loop() {
  // Actualizar todas las tareas (es lo único no bloqueante)
  tareaSensores.Update();
  tareaLCD.Update();
  tareaTeclado.Update();
  tareaAlarmas.Update();

  // Actualizar la máquina de estados
  loopFSM();
}