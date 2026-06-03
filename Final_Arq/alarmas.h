#ifndef ALARMAS_H
#define ALARMAS_H

#include <Arduino.h>

void inicializarAlarmas();
void verificarAlarma(float temperatura, float humedad, int presion);
bool hayEmergencia();

#endif