/**
 * @file fsm.h
 * @brief Declaración de la máquina de estados finitos (FSM) del sistema.
 * @details Define los estados del sistema y las funciones para inicializar,
 *          ejecutar y enviar eventos a la FSM.
 */

#ifndef FSM_H
#define FSM_H

#include <Arduino.h>

/**
 * @enum EstadoSistema
 * @brief Posibles estados del sistema de confort térmico.
 */
enum EstadoSistema {
  ESTADO_INICIO,            ///< Estado inicial: espera tarjeta RFID
  ESTADO_IDENTIFICACION,    ///< Leyendo y validando tarjeta
  ESTADO_SELECCION_MODO,    ///< Menú principal: automático/manual/configuración
  ESTADO_REGULACION_AUTO,   ///< Control automático de temperatura
  ESTADO_REGULACION_MANUAL, ///< Usuario controla actuadores manualmente
  ESTADO_CONFIGURACION,     ///< Ajuste de parámetros (setpoints, usuarios)
  ESTADO_ALARMA_SIMPLE,     ///< Alarma no crítica, se puede retornar a regulación
  ESTADO_EMERGENCIA         ///< Estado crítico por 3 alarmas en 12s
};

/**
 * @brief Inicializa la máquina de estados.
 * @details Debe llamarse una vez en setup().
 */
void setupFSM();

/**
 * @brief Actualiza la máquina de estados.
 * @details Debe invocarse repetidamente en loop().
 *          Evalúa transiciones basadas en eventos y ejecuta acciones de estado.
 */
void loopFSM();

/**
 * @brief Dispara un evento externo hacia la FSM.
 * @param evento Código numérico del evento (definir según necesidades).
 */
void dispararEvento(int evento);

#endif // FSM_H