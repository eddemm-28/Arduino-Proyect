#include "alarmas.h"

static int alarmasConsecutivas = 0;
static unsigned long tiempoUltimaAlarma = 0;
static bool emergenciaActiva = false;

void inicializarAlarmas() {
  alarmasConsecutivas = 0;
  emergenciaActiva = false;
}

void verificarAlarma(float temperatura, float humedad, int presion) {
  // Detectar condiciones anormales
  bool condicionPeligro = (temperatura > 35.0 || temperatura < 10.0);
  if (condicionPeligro) {
    // Lógica de cuenta de alarmas en ventana de 12 segundos
  } else {
    // Reiniciar contador si pasa tiempo suficiente sin alarma
  }
}

bool hayEmergencia() {
  return emergenciaActiva;
}