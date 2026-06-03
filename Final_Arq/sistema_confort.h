#ifndef SISTEMA_CONFORT_H
#define SISTEMA_CONFORT_H

#include <Arduino.h>
#include <LiquidCrystal_I2C.h>
#include <Keypad.h>
#include <DHT.h>

class SistemaConfort {
  public:
    SistemaConfort();
    void begin();
    void leerSensores();      // Lee DHT, LDR, presión (sin delay)
    void actualizarLCD();     // Actualiza la pantalla con valores actuales
    void leerTeclado();       // Lee teclado y almacena tecla presionada
    void controlarAlarmas();  // Evalúa condiciones de alarma y cuenta en ventana de 12s
    
    // Getters para que la FSM pueda consultar el estado
    float getTemperatura() { return temperatura; }
    float getHumedad() { return humedad; }
    int getLuz() { return luz; }
    int getPresion() { return presion; }
    int getAlarmasConsecutivas() { return alarmasConsecutivas; }
    bool hayEmergencia() { return emergenciaActiva; }

  private:
    void leerDHT();
    void leerLDR();
    void leerPresion();
    void procesarTecla(char tecla);
    
    // Variables de sensores
    float temperatura;
    float humedad;
    int luz;
    int presion;
    
    // Variables para alarmas (ventana de 12 segundos)
    int alarmasConsecutivas;
    unsigned long tiempoPrimeraAlarma;
    bool emergenciaActiva;
    
    // Última tecla presionada (para pasar a la FSM)
    char ultimaTecla;
    
    // Objetos de periféricos
    LiquidCrystal_I2C lcd;
    DHT dht;
    Keypad teclado;
};

#endif