#include "sistema_confort.h"
#include "configuracion.h"

// Definición de pines del teclado (deben coincidir con configuracion.h)
const byte filasKeypad[FILAS_KEYPAD] = {26, 27, 28, 29};
const byte columnasKeypad[COLUMNAS_KEYPAD] = {30, 31, 32, 33};
const char teclasMap[FILAS_KEYPAD][COLUMNAS_KEYPAD] = {
  {'1','2','3','A'},
  {'4','5','6','B'},
  {'7','8','9','C'},
  {'*','0','#','D'}
};

// Constructor: inicializa variables y objetos
SistemaConfort::SistemaConfort() 
  : lcd(0x27, 16, 2), dht(PIN_DHT, DHT11), 
    teclado(makeKeymap(teclasMap), filasKeypad, columnasKeypad, FILAS_KEYPAD, COLUMNAS_KEYPAD) {
  temperatura = 0;
  humedad = 0;
  luz = 0;
  presion = 0;
  alarmasConsecutivas = 0;
  tiempoPrimeraAlarma = 0;
  emergenciaActiva = false;
  ultimaTecla = 0;
}

void SistemaConfort::begin() {
  Serial.begin(9600);
  lcd.init();
  lcd.backlight();
  dht.begin();
  
  pinMode(PIN_LDR, INPUT);
  pinMode(PIN_PRESION, INPUT);
  pinMode(PIN_MOTOR, OUTPUT);
  pinMode(PIN_LED_ROJO, OUTPUT);
  pinMode(PIN_LED_VERDE, OUTPUT);
  pinMode(PIN_LED_AZUL, OUTPUT);
  pinMode(PIN_BUZZER, OUTPUT);
  
  // Apagar todo al inicio
  digitalWrite(PIN_MOTOR, LOW);
  digitalWrite(PIN_LED_ROJO, LOW);
  digitalWrite(PIN_LED_VERDE, LOW);
  digitalWrite(PIN_LED_AZUL, LOW);
  digitalWrite(PIN_BUZZER, LOW);
  
  lcd.print("Sistema listo");
  delay(2000);
  lcd.clear();
}

void SistemaConfort::leerSensores() {
  leerDHT();
  leerLDR();
  leerPresion();
}

void SistemaConfort::leerDHT() {
  float t = dht.readTemperature();
  float h = dht.readHumidity();
  if (!isnan(t)) temperatura = t;
  if (!isnan(h)) humedad = h;
}

void SistemaConfort::leerLDR() {
  luz = analogRead(PIN_LDR);
}

void SistemaConfort::leerPresion() {
  presion = analogRead(PIN_PRESION);
}

void SistemaConfort::actualizarLCD() {
  lcd.setCursor(0, 0);
  lcd.print("T:");
  lcd.print(temperatura, 1);
  lcd.print("C H:");
  lcd.print(humedad, 0);
  lcd.print("% ");
  
  lcd.setCursor(0, 1);
  lcd.print("Luz:");
  lcd.print(map(luz, 0, 1023, 0, 100));
  lcd.print("% ");
  
  if (emergenciaActiva) {
    lcd.print("EMERG!");
  } else if (alarmasConsecutivas > 0) {
    lcd.print("ALARM");
  } else {
    lcd.print("     ");
  }
}

void SistemaConfort::leerTeclado() {
  char tecla = teclado.getKey();
  if (tecla) {
    ultimaTecla = tecla;
    procesarTecla(tecla);
  }
}

void SistemaConfort::procesarTecla(char tecla) {
  // Aquí puedes enviar la tecla a la FSM, o almacenarla en una variable global
  Serial.print("Tecla presionada: ");
  Serial.println(tecla);
  // Por ahora solo la mostramos; después conectaremos con la FSM
}

void SistemaConfort::controlarAlarmas() {
  // Definir condición de alarma: temperatura > 35°C o < 10°C
  bool condicionPeligro = (temperatura > 35.0 || temperatura < 10.0);
  
  static bool alarmaAnterior = false;
  static unsigned long inicioVentana = 0;
  
  if (condicionPeligro && !alarmaAnterior) {
    // Nueva alarma detectada
    if (alarmasConsecutivas == 0) {
      // Primera alarma: iniciar ventana de 12 segundos
      inicioVentana = millis();
      tiempoPrimeraAlarma = inicioVentana;
      alarmasConsecutivas = 1;
    } else {
      // Si la alarma ocurre dentro de los 12 segundos desde la primera, incrementar contador
      if (millis() - inicioVentana <= 12000) {
        alarmasConsecutivas++;
      } else {
        // Reiniciar ventana si pasaron más de 12s
        inicioVentana = millis();
        alarmasConsecutivas = 1;
      }
    }
    
    // Si alcanzamos 3 alarmas en menos de 12s, activar emergencia
    if (alarmasConsecutivas >= 3 && (millis() - inicioVentana <= 12000)) {
      emergenciaActiva = true;
      // Acciones de emergencia: apagar motor, encender led rojo, buzzer continuo
      digitalWrite(PIN_MOTOR, LOW);
      digitalWrite(PIN_LED_ROJO, HIGH);
      tone(PIN_BUZZER, 2500);
      Serial.println("!!! EMERGENCIA: 3 ALARMAS EN 12 SEGUNDOS !!!");
    } else {
      // Alarma simple: led rojo parpadeante y buzzer corto
      static unsigned long ultimoParpadeo = 0;
      if (millis() - ultimoParpadeo > 500) {
        ultimoParpadeo = millis();
        digitalWrite(PIN_LED_ROJO, !digitalRead(PIN_LED_ROJO));
        digitalWrite(PIN_BUZZER, !digitalRead(PIN_BUZZER));
      }
    }
    alarmaAnterior = true;
  } else if (!condicionPeligro) {
    // Sin peligro: apagar led rojo y buzzer, pero mantener emergencia hasta reset manual
    if (!emergenciaActiva) {
      digitalWrite(PIN_LED_ROJO, LOW);
      digitalWrite(PIN_BUZZER, LOW);
    }
    // Si no hay peligro, reiniciamos el contador solo si pasó la ventana
    if (alarmasConsecutivas > 0 && (millis() - inicioVentana > 12000)) {
      alarmasConsecutivas = 0;
    }
    alarmaAnterior = false;
  }
}