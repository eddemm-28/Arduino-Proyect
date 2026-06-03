#include "configuracion.h"
#include "sistema_confort.h"
#include "fsm.h"
#include "alarmas.h"

// Objeto global del sistema
SistemaConfort confort;

void setup() {
  confort.begin();
  setupFSM();
  inicializarAlarmas();
  Serial.println("Sistema iniciado");
}

void loop() {
  // Actualizar tareas asincrónicas (sin delay)
  confort.tickSensores();
  confort.tickLCD();
  confort.tickTeclado();
  confort.tickAlarmas();
  
  // Actualizar máquina de estados
  loopFSM();
}