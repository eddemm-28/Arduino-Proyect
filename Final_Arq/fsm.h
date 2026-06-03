#ifndef FSM_H
#define FSM_H

#include <Arduino.h>

enum EstadoSistema {
  ESTADO_INICIO,
  ESTADO_IDENTIFICACION,
  ESTADO_SELECCION_MODO,
  ESTADO_REGULACION_AUTO,
  ESTADO_REGULACION_MANUAL,
  ESTADO_CONFIGURACION,
  ESTADO_ALARMA_SIMPLE,
  ESTADO_ALARMA_CRITICA
};

void setupFSM();
void loopFSM();
void dispararEvento(int evento);

#endif