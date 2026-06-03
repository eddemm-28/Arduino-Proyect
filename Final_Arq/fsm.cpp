/**
 * @file fsm.cpp
 * @brief Implementación de la máquina de estados finitos.
 * @details Por ahora es un esqueleto básico. Se completará a medida que
 *          se defina la lógica de navegación e interacción con el teclado/RFID.
 */

#include "fsm.h"
#include "configuracion.h"

static EstadoSistema estadoActual = ESTADO_INICIO; ///< Estado actual del sistema

void setupFSM() {
  estadoActual = ESTADO_INICIO;
  Serial.println(F("FSM inicializada en ESTADO_INICIO"));
}

void loopFSM() {
  // Aquí se implementará la lógica completa de la FSM
  // Por ahora, solo se muestra el estado actual cada cierto tiempo (debug)
  static unsigned long ultimoDebug = 0;
  if (millis() - ultimoDebug > 5000) {
    ultimoDebug = millis();
    Serial.print(F("Estado actual: "));
    switch(estadoActual) {
      case ESTADO_INICIO: Serial.println(F("INICIO")); break;
      case ESTADO_IDENTIFICACION: Serial.println(F("IDENTIFICACION")); break;
      case ESTADO_SELECCION_MODO: Serial.println(F("SELECCION_MODO")); break;
      case ESTADO_REGULACION_AUTO: Serial.println(F("REGULACION_AUTO")); break;
      case ESTADO_REGULACION_MANUAL: Serial.println(F("REGULACION_MANUAL")); break;
      case ESTADO_CONFIGURACION: Serial.println(F("CONFIGURACION")); break;
      case ESTADO_ALARMA_SIMPLE: Serial.println(F("ALARMA_SIMPLE")); break;
      case ESTADO_EMERGENCIA: Serial.println(F("EMERGENCIA")); break;
      default: Serial.println(F("DESCONOCIDO"));
    }
  }
  
  // Ejemplo de transición simple (se puede conectar con teclado o RFID)
  // if (estadoActual == ESTADO_INICIO && tarjetaPresente) estadoActual = ESTADO_IDENTIFICACION;
}

void dispararEvento(int evento) {
  // En el futuro, esta función cambiará el estado según el evento recibido
  Serial.print(F("Evento recibido: "));
  Serial.println(evento);
}