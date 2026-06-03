/**
 * @file alarmas.h
 * @brief Declaración de funciones para la gestión de alarmas.
 * @details Proporciona una interfaz simple para inicializar, verificar
 *          condiciones de alarma y consultar si el sistema está en emergencia.
 */

#ifndef ALARMAS_H
#define ALARMAS_H

#include <Arduino.h>

/**
 * @brief Inicializa el sistema de alarmas (contadores y banderas).
 */
void inicializarAlarmas();

/**
 * @brief Verifica condiciones de peligro y actualiza el estado de alarmas.
 * @param temperatura Temperatura actual en °C.
 * @param humedad Parámetro obsoleto (no usado, se mantiene por compatibilidad).
 * @param presion Parámetro obsoleto (no usado).
 * @note En esta versión solo se usa la temperatura; la humedad y presión
 *       se ignoran (se pueden eliminar en futuras revisiones).
 */
void verificarAlarma(float temperatura, float humedad, int presion);

/**
 * @brief Indica si el sistema se encuentra en estado de emergencia.
 * @return true si hay emergencia activa, false en caso contrario.
 */
bool hayEmergencia();

#endif // ALARMAS_H