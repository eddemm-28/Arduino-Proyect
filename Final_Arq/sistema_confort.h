#ifndef SISTEMA_CONFORT_H
#define SISTEMA_CONFORT_H

#include <Arduino.h>
#include "AsyncTaskLib.h"
#include "configuracion.h"

class SistemaConfort {
  public:
    SistemaConfort();
    void begin();
    void tickSensores();   // tarea asincrónica para sensores
    void tickLCD();        // tarea asincrónica para LCD
    void tickTeclado();    // tarea asincrónica para teclado
    void tickAlarmas();    // tarea asincrónica para alarmas

    // Datos actuales
    float getTemperatura();
    float getHumedad();
    int getLuz();
    int getPresion();

  private:
    void leerDHT();
    void leerLDR();
    void leerPresion();
    void actualizarLCD();
    void procesarTecla(char tecla);
    void evaluarAlarmas();

    unsigned long tiempoUltimoSensor;
    unsigned long tiempoUltimoLCD;
    unsigned long tiempoUltimoTeclado;
    unsigned long tiempoUltimoAlarma;
};

#endif