int sensorPin = A1;   // interface pin with magnetic sensor
int val;              // variable to store read values

void setup() {
  pinMode(A0, INPUT);   // set analog pin as input
  Serial.begin(9600);   // initialize serial interface
}

void loop() {
  val = analogRead(sensorPin);  // read sensor value
  Serial.println(val);          // print value to serial

  delay(100);  
}