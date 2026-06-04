/**
 * @file sistema_confort.h
 * @brief Declaración de la clase principal SistemaConfort.
 */

#ifndef SISTEMA_CONFORT_H
#define SISTEMA_CONFORT_H

#include <Arduino.h>
#include <LiquidCrystal.h>
#include <Keypad.h>
#include <Servo.h>
#include <EEPROM.h>
#include <MFRC522.h>
#include "configuracion.h"

class SistemaConfort {
  public:
    SistemaConfort();
    void begin();
    void leerSensores();
    void actualizarLCD();
    void leerTeclado();
    void controlarAlarmas();
    void testHardware();
    void leerBoton();
    bool leerRFID();
    void guardarCredenciales(String clave, String uid);
    bool validarClave(String entrada);
    bool validarUID(String uid);
    int getIntentosFallidos();
    void incrementarIntentosFallidos();
    void resetIntentosFallidos();

    // Getters para la FSM
    float getTemperatura() const { return temperatura; }
    int getLuz() const { return luz; }
    int getCampoMagnetico() const { return campoMagnetico; }
    int getSonidoAnalog() const { return sonidoAnalog; }
    bool getSonidoDigital() const { return sonidoDigital; }
    int getAlarmasConsecutivas() const { return alarmasConsecutivas; }
    bool hayEmergencia() const { return emergenciaActiva; }
    LiquidCrystal& getLCD() { return lcd; }
    String getClaveAlmacenada() const { return claveAlmacenada; }
    String getUIDAlmacenado() const { return uidAlmacenado; }
    String leerCualquierRFID();   // Lee UID sin validar

  private:
    void leerTermistor();
    void leerLDR();
    void leerHall();
    void leerSonido();
    void procesarTecla(char tecla);
    void cargarCredencialesDesdeEEPROM();   // <-- AGREGAR ESTA DECLARACIÓN

    // Variables de sensores
    float temperatura;
    int luz;
    int campoMagnetico;
    int sonidoAnalog;
    bool sonidoDigital;

    // Variables de alarmas (estas faltaban)
    int alarmasConsecutivas;
    unsigned long tiempoPrimeraAlarma;
    bool emergenciaActiva;

    char ultimaTecla;

    // Objetos de periféricos
    LiquidCrystal lcd;
    Keypad teclado;
    Servo myservo;

    // Funcionalidades
    int intentosFallidos;
    String claveAlmacenada;
    String uidAlmacenado;
    MFRC522 rfid;
};

#endif // SISTEMA_CONFORT_H