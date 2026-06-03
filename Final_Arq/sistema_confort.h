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
#include <LiquidCrystal_I2C.h>
#include <Keypad.h>
#include <Servo.h>
#include "configuracion.h"

/**
 * @class SistemaConfort
 * @brief Clase principal que controla todo el sistema de confort térmico.
 */
class SistemaConfort {
  public:
    /**
     * @brief Constructor. Inicializa variables internas y objetos de periféricos.
     */
    SistemaConfort();

    /**
     * @brief Inicializa pines, LCD, servo y periféricos.
     * @details Debe llamarse una vez en setup().
     */
    void begin();

    /**
     * @brief Lee todos los sensores (termistor, LDR, Hall, sonido) sin bloquear.
     * @details Está diseñada para ser llamada periódicamente desde una AsyncTask.
     */
    void leerSensores();

    /**
     * @brief Actualiza la pantalla LCD con los últimos valores de sensores y estado.
     * @details Se invoca desde una tarea asincrónica cada 500 ms.
     */
    void actualizarLCD();

    /**
     * @brief Escanea el teclado matricial y procesa la tecla presionada.
     * @details Se ejecuta cada 100 ms desde una AsyncTask.
     */
    void leerTeclado();

    /**
     * @brief Evalúa condiciones de alarma y gestiona la ventana de 12 segundos.
     * @details Detecta peligro por temperatura o sonido, incrementa contador
     *          y activa emergencia si se alcanzan 3 alarmas en menos de 12s.
     *          Se ejecuta cada 1 segundo desde una tarea asincrónica.
     */
    void controlarAlarmas();

    /**
     * @brief Prueba secuencial de todos los periféricos (LCD, sensores, servo, LEDs, buzzer).
     * @details Muestra resultados en LCD y Serial. Útil para validar el hardware.
     */
    void testHardware();

    // Getters para la FSM
    float getTemperatura() const { return temperatura; }
    int getLuz() const { return luz; }
    int getCampoMagnetico() const { return campoMagnetico; }
    int getSonidoAnalog() const { return sonidoAnalog; }
    bool getSonidoDigital() const { return sonidoDigital; }
    int getAlarmasConsecutivas() const { return contadorAlarmas; }
    bool hayEmergencia() const { return emergenciaActiva; }

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
    LiquidCrystal_I2C lcd;    ///< Objeto para controlar LCD I2C
    Keypad teclado;           ///< Objeto para teclado matricial
    Servo myservo;            ///< Objeto para servomotor
};

#endif // SISTEMA_CONFORT_H