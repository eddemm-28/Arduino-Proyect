#ifndef FSM_H
#define FSM_H

#include <Arduino.h>
#include "sistema_confort.h"

enum EstadoSistema {
  ESTADO_INICIO,
  ESTADO_BLOQUEO,
  ESTADO_CONFIGURACION,
  ESTADO_MONITOR_INTRUSOS,
  ESTADO_MONITOR_AMBIENTAL,
  ESTADO_ALARMA
};

// Eventos que pueden disparar transiciones
enum EventoFSM {
  EVENTO_CLAVE_CORRECTA,
  EVENTO_CLAVE_INCORRECTA,
  EVENTO_BOTON_RESET,
  EVENTO_TECLA_HASH,      // '#'
  EVENTO_TECLA_ASTERISCO, // '*'
  EVENTO_TECLA_A,         // Para salir de config a monitoreo
  EVENTO_SONIDO_ALTO,     // Micrófono supera umbral
  EVENTO_CONDICION_ALARMA_AMBIENTAL, // temp<20 y luz<100
  EVENTO_TIMER_2S,        // Transición 4->5
  EVENTO_TIMER_5S,        // Transición 5->4
  EVENTO_TIMER_2S_DESDE_ALARMA,
  EVENTO_TIMER_4S_DESDE_ALARMA,
  EVENTO_TRES_ALARMAS_EN_12S,
  EVENTO_RFID_DETECTADO
};

void setupFSM();
void loopFSM();
void dispararEvento(int evento);
EstadoSistema getEstadoActual();
void actualizarLEDyBuzzer();  // Controla LEDs y buzzer según estado

// Para que la FSM pueda acceder a los sensores y al sistema
extern SistemaConfort *ptrSistema;  // Se declarará en Final_Arq.ino

enum SubEstadoConfig {
  CONFIG_MENU,
  CONFIG_CAMBIO_CLAVE,
  CONFIG_CONFIRMAR_CLAVE,
  CONFIG_REGISTRO_RFID
};
#endif