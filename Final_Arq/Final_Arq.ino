/**
 * @file Final_Arq.ino
 * @brief Programa principal del Sistema de Confort Térmico.
 * @details Integra todos los módulos (sensores, actuadores, FSM, alarmas)
 *          y utiliza la librería AsyncTask para ejecutar tareas no bloqueantes.
 * @version 1.0
 */

#include "configuracion.h"
#include "sistema_confort.h"
#include "fsm.h"
#include "alarmas.h"
#include "AsyncTaskLib.h"
#include <EEPROM.h>
#include <MFRC522.h>

// ==================== DECLARACIÓN DE CALLBACKS ====================
void callbackLeerSensores();
void callbackLeerTeclado();

// ==================== OBJETOS GLOBALES ====================
SistemaConfort confort;
bool botonPresionado = false;
unsigned long tiempoUltimoBoton = 0;
String inputBuffer = "";
bool bufferCompleto = false;

void limpiarBuffer() { inputBuffer = ""; bufferCompleto = false; }
String obtenerBufferEntrada() { return inputBuffer; }

String ultimoUIDLeido = "";
String obtenerUIDLeido() { return ultimoUIDLeido; }

// ==================== TAREAS ASINCRÓNICAS ====================
AsyncTask tareaSensores(2000, true, callbackLeerSensores);
// *** CORRECCIÓN: tareaLCD eliminada. ***
// confort.actualizarLCD() estaba sobreescribiendo el LCD de la FSM y además
// generaba lcd.clear() adicionales que bloqueaban la lectura del teclado.
// La FSM gestiona el LCD directamente en actualizarLCDporEstado() con su
// propio control de tiempo (cada 300ms).
AsyncTask tareaTeclado(100, true, callbackLeerTeclado);

// ==================== CALLBACKS ====================
void callbackLeerSensores() {
  confort.leerSensores();
}

void callbackLeerTeclado() {
  confort.leerTeclado();
}

// ==================== SETUP ====================
void setup() {
  confort.begin();
  setupFSM();
  ptrSistema = &confort;
  inicializarAlarmas();

  tareaSensores.Start();
  tareaTeclado.Start();

  pinMode(PIN_LED_ALARMA, OUTPUT);
  Serial.println(F("Sistema iniciado. Tareas asincronicas corriendo."));
}

// ==================== LOOP ====================
void loop() {
  tareaSensores.Update();
  tareaTeclado.Update();
  loopFSM();
  
  static unsigned long lastCheck = 0;
  if (millis() - lastCheck > 500) {
    lastCheck = millis();
    
    if (getEstadoActual() == ESTADO_MONITOR_INTRUSOS) {
      // Sonido fuerte
      if (confort.getSonidoAnalog() > SONIDO_UMBRAL) {
        dispararEvento(EVENTO_SONIDO_ALTO);
      }
      // Campo magnético: el sensor Hall devuelve ~512 en reposo;
      // una variación > 50 respecto al centro indica presencia de imán
      int hall = confort.getCampoMagnetico();
      if (hall < 500 || hall > 540) {
        dispararEvento(EVENTO_HALL_DETECTADO);
      }
    }
    else if (getEstadoActual() == ESTADO_MONITOR_AMBIENTAL) {
      // Temperatura menor a 20°C dispara alarma ambiental
      if (confort.getTemperatura() < 20.0) {
        dispararEvento(EVENTO_CONDICION_ALARMA_AMBIENTAL);
      }
    }
    else if (getEstadoActual() == ESTADO_INICIO) {
      if (bufferCompleto) {
        if (confort.validarClave(inputBuffer)) {
          dispararEvento(EVENTO_CLAVE_CORRECTA);
        } else {
          confort.incrementarIntentosFallidos();
          Serial.print("Intento fallido #");
          Serial.println(confort.getIntentosFallidos());
          dispararEvento(EVENTO_CLAVE_INCORRECTA);
        }
        limpiarBuffer();
      }
      if (confort.leerRFID()) {
        dispararEvento(EVENTO_CLAVE_CORRECTA);
      }
    }
    else if (getEstadoActual() == ESTADO_CONFIGURACION) {
      if (getSubEstadoConfig() == CONFIG_REGISTRO_RFID) {
        String uid = confort.leerCualquierRFID();
        if (uid != "") {
          ultimoUIDLeido = uid;
          dispararEvento(EVENTO_RFID_DETECTADO);
        }
      }
    }
  }
}
