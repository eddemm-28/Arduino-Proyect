#include "sistema_confort.h"
#include "configuracion.h"

// Definición de pines del teclado (EJEMPLO REAL)
const byte filasKeypad[FILAS_KEYPAD] = {32, 33, 34, 35};
const byte columnasKeypad[COLUMNAS_KEYPAD] = {A7, A6, A5, A4};
const char teclasMap[FILAS_KEYPAD][COLUMNAS_KEYPAD] = {
  {'1','2','3','A'},
  {'4','5','6','B'},
  {'7','8','9','C'},
  {'*','0','#','D'}
};

SistemaConfort::SistemaConfort() 
  : lcd(0x27, 16, 2), dht(PIN_DHT, DHT11), 
    teclado(makeKeymap(teclasMap), filasKeypad, columnasKeypad, FILAS_KEYPAD, COLUMNAS_KEYPAD) {
  temperatura = 0;
  humedad = 0;
  luz = 0;
  hall = 0;
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
  pinMode(PIN_HALL, INPUT);
  myservo.attach(PIN_MOTOR);      // Adjuntar servo al pin 13
  myservo.write(0);               // Posición inicial cerrada
  pinMode(PIN_LED_ALARMA, OUTPUT);
  pinMode(PIN_LED_RGB_R, OUTPUT);
  pinMode(PIN_LED_RGB_G, OUTPUT);
  pinMode(PIN_LED_RGB_B, OUTPUT);
  pinMode(PIN_BUZZER, OUTPUT);
  
  // Apagar todo al inicio
  digitalWrite(PIN_LED_ALARMA, LOW);
  digitalWrite(PIN_LED_RGB_R, LOW);
  digitalWrite(PIN_LED_RGB_G, LOW);
  digitalWrite(PIN_LED_RGB_B, LOW);
  digitalWrite(PIN_BUZZER, LOW);
  
  lcd.print("Sistema listo");
  delay(2000);
  lcd.clear();
}

void SistemaConfort::leerSensores() {
  leerDHT();
  leerLDR();
  leerHall();   // ahora lee sensor Hall en lugar de presión
}

void SistemaConfort::leerDHT() {
  float t = dht.readTemperature();
  float h = dht.readHumidity();
  if (!isnan(t)) temperatura = t;
  if (!isnan(h)) humedad = h;
}

void SistemaConfort::leerLDR() {
  luz = analogRead(PIN_LDR);  // A3
}

void SistemaConfort::leerHall() {
  hall = analogRead(PIN_HALL); // A1
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
  lcd.print("% Hall:");
  lcd.print(hall);          // Muestra el valor del sensor Hall
  lcd.print("   ");
  
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
  bool condicionPeligro = (temperatura > 35.0 || temperatura < 10.0);
  
  static bool alarmaAnterior = false;
  static unsigned long inicioVentana = 0;
  
  if (condicionPeligro && !alarmaAnterior) {
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
      emergenciaActiva = true;
      myservo.write(0);               // Cerrar puerta en emergencia
      digitalWrite(PIN_LED_ALARMA, HIGH);
      tone(PIN_BUZZER, 2500);
      Serial.println("!!! EMERGENCIA: 3 ALARMAS EN 12 SEGUNDOS !!!");
    } else {
      // Alarma simple: parpadeo con LED de alarma (pin 3) y buzzer
      static unsigned long ultimoParpadeo = 0;
      if (millis() - ultimoParpadeo > 500) {
        ultimoParpadeo = millis();
        digitalWrite(PIN_LED_ALARMA, !digitalRead(PIN_LED_ALARMA));
        digitalWrite(PIN_BUZZER, !digitalRead(PIN_BUZZER));
      }
    }
    alarmaAnterior = true;
  } else if (!condicionPeligro) {
    if (!emergenciaActiva) {
      digitalWrite(PIN_LED_ALARMA, LOW);
      digitalWrite(PIN_BUZZER, LOW);
    }
    if (alarmasConsecutivas > 0 && (millis() - inicioVentana > 12000)) {
      alarmasConsecutivas = 0;
    }
    alarmaAnterior = false;
  }
}