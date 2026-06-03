#include "fsm.h"

static EstadoSistema estadoActual = ESTADO_INICIO;

void setupFSM() {
  estadoActual = ESTADO_INICIO;
}

void loopFSM() {
  // Lógica de la máquina de estados (se implementará después)
  switch(estadoActual) {
    case ESTADO_INICIO:
      // transiciones
      break;
    // ... otros casos
    default: break;
  }
}

void dispararEvento(int evento) {
  // Se usará para cambiar de estado según eventos
}