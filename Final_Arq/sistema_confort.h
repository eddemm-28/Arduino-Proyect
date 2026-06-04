/**
 * @file sistema_confort.h
 * @brief Declaración de la clase principal SistemaConfort.
 * @details Encapsula la lógica de sensores, actuadores, tareas asincrónicas
 *          y la interfaz de usuario (LCD, teclado). Proporciona métodos para
 *          leer sensores, actualizar pantalla, gestionar alarmas y probar hardware.
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


/**
 * @class SistemaConfort
 * @brief Clase principal que controla todo el sistema de confort térmico.
 */
class SistemaConfort {
  public:
    SistemaConfort();
    void begin();
    void leerSensores();
    void actualizarLCD();   // Ya no se usará, se mantiene por compatibilidad
    void leerTeclado();
    void controlarAlarmas(); // Ya no se usará, se mantiene por compatibilidad
    void testHardware();     // Se puede eliminar o comentar su uso
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
    int getAlarmasConsecutivas() const { return contadorAlarmas; }
    bool hayEmergencia() const { return emergenciaActiva; }
    LiquidCrystal& getLCD() { return lcd; }  // <- Nuevo getter

  private:
    // Métodos privados de lectura de sensores
    void leerTermistor();     ///< Lee y calcula temperatura mediante termistor NTC
    void leerLDR();           ///< Lee valor de luz analógico
    void leerHall();          ///< Lee sensor magnético Hall
    void leerSonido();        ///< Lee sensor KY-037 (digital y analógico)

    void procesarTecla(char tecla); ///< Procesa la tecla (enlace con FSM)

    // Variables de sensores
    float temperatura;        ///< Temperatura actual en °C
    int luz;                  ///< Valor de luminosidad (0-1023)
    int campoMagnetico;       ///< Valor del sensor Hall (0-1023)
    int sonidoAnalog;         ///< Valor analógico del sonido (0-1023)
    bool sonidoDigital;       ///< Estado digital del sonido (HIGH/LOW)

    // Variables de alarmas
    int alarmasConsecutivas;  ///< Contador de alarmas en ventana de 12s
    unsigned long tiempoPrimeraAlarma; ///< Tiempo de inicio de ventana
    bool emergenciaActiva;    ///< Indica si el sistema está en emergencia

    char ultimaTecla;         ///< Última tecla presionada

    // Objetos de periféricos
    LiquidCrystal lcd;   ///< Objeto para controlar LCD I2C
    Keypad teclado;           ///< Objeto para teclado matricial
    Servo myservo;            ///< Objeto para servomotor

    //Funcionalidades
    int intentosFallidos;                 ///< Para bloqueo tras 3 fallos
    String claveAlmacenada;               ///< Cache de la clave desde EEPROM
    String uidAlmacenado;                 ///< Cache del UID
    MFRC522 rfid;                         ///< Objeto RFID (declarar después de incluir librería)
};

#endif // SISTEMA_CONFORT_H