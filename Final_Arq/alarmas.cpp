/**
 * @file alarmas.cpp
 * @brief Implementación de la gestión de alarmas.
 * @details Mantiene contadores y la lógica de ventana de 12 segundos.
 *          Actualmente depende de la variable global 'contadorAlarmas'
 *          y 'emergenciaActiva' definidas en configuracion.h.
 */

#include "alarmas.h"
#include "configuracion.h"

static int alarmasConsecutivas = 0;      ///< Contador local (podría unificarse con variable global)
static unsigned long tiempoUltimaAlarma = 0; ///< Momento de la última alarma
static bool emergenciaActivaLocal = false;   ///< Bandera local de emergencia

void inicializarAlarmas() {
  alarmasConsecutivas = 0;
  emergenciaActivaLocal = false;
  // También reiniciamos las variables globales
  contadorAlarmas = 0;
  emergenciaActiva = false;
  tiempoPrimeraAlarma = 0;
}

void verificarAlarma(float temperatura, float humedad, int presion) {
  // Se ignora humedad y presion (parámetros legacy)
  bool condicionPeligro = (temperatura > TEMP_MAX || temperatura < TEMP_MIN);
  
  static bool alarmaPrevia = false;
  static unsigned long inicioVentana = 0;
  
  if (condicionPeligro && !alarmaPrevia) {
    // Nueva alarma
    if (alarmasConsecutivas == 0) {
      inicioVentana = millis();
      tiempoPrimeraAlarma = inicioVentana;
      alarmasConsecutivas = 1;
    } else {
      if (millis() - inicioVentana <= 12000) {
        alarmasConsecutivas++;
      } else {
        inicioVentana = millis();
        alarmasConsecutivas = 1;
      }
    }
    
    if (alarmasConsecutivas >= 3 && (millis() - inicioVentana <= 12000)) {
      emergenciaActivaLocal = true;
      emergenciaActiva = true;
      Serial.println(F("EMERGENCIA por 3 alarmas en 12s (desde alarmas.cpp)"));
    }
    alarmaPrevia = true;
  } else if (!condicionPeligro) {
    if (alarmasConsecutivas > 0 && (millis() - inicioVentana > 12000)) {
      alarmasConsecutivas = 0;
      emergenciaActivaLocal = false;
      emergenciaActiva = false;
    }
    alarmaPrevia = false;
  }
  
  // Actualizar variable global para que otros módulos la vean
  contadorAlarmas = alarmasConsecutivas;
}

bool hayEmergencia() {
  return emergenciaActivaLocal || emergenciaActiva;
}