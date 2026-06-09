#ifndef FSM_H
#define FSM_H
 
#include <Arduino.h>
#include "sistema_confort.h"
 
// ==================== ENUMS PRINCIPALES ====================
enum EstadoSistema {
  ESTADO_INGRESAR_HORA,   // *** NUEVO: pedir hora antes del login ***
  ESTADO_INICIO,
  ESTADO_BLOQUEO,
  ESTADO_CONFIGURACION,
  ESTADO_MONITOR_INTRUSOS,
  ESTADO_MONITOR_AMBIENTAL,
  ESTADO_ALARMA
};
 
enum EventoFSM {
  EVENTO_CLAVE_CORRECTA,
  EVENTO_CLAVE_INCORRECTA,
  EVENTO_BOTON_RESET,
  EVENTO_TECLA_HASH,
  EVENTO_TECLA_ASTERISCO,
  EVENTO_TECLA_A,
  EVENTO_SONIDO_ALTO,
  EVENTO_HALL_DETECTADO,           // <-- NUEVO: campo magnético detectado
  EVENTO_CONDICION_ALARMA_AMBIENTAL,
  EVENTO_TIMER_2S,
  EVENTO_TIMER_5S,
  EVENTO_TIMER_2S_DESDE_ALARMA,
  EVENTO_TIMER_4S_DESDE_ALARMA,
  EVENTO_TRES_ALARMAS_EN_12S,
  EVENTO_RFID_DETECTADO
};
 
enum SubEstadoConfig {
  CONFIG_MENU,
  CONFIG_CAMBIO_CLAVE,
  CONFIG_CONFIRMAR_CLAVE,
  CONFIG_REGISTRO_RFID,
  CONFIG_FRANJAS_MENU,        // *** NUEVO ***
  CONFIG_FRANJAS_HORA_INICIO, // *** NUEVO ***
  CONFIG_FRANJAS_HORA_FIN     // *** NUEVO ***
};
 
// ==================== PROTOTIPOS DE FUNCIONES ====================
void setupFSM();
void loopFSM();
void dispararEvento(int evento);
EstadoSistema getEstadoActual();
SubEstadoConfig getSubEstadoConfig();
void actualizarLEDyBuzzer();
 
extern SistemaConfort *ptrSistema;
 
#endif
 