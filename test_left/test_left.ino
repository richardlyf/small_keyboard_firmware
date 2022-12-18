#include <Wire.h>

bool pressed;

void setup() {
  // put your setup code here, to run once:
  Wire.begin(4);
  Wire.setClock(400000);
  digitalWrite(SDA, LOW);
  digitalWrite(SCL, LOW);
  pinMode(A1, INPUT_PULLUP);
  pinMode(13, OUTPUT);
  Wire.onRequest(handler);
  Serial.begin(9600);
}

void handler() {
  if (pressed) {
    Wire.write('Y');
    digitalWrite(13, HIGH);
  } else {
    Wire.write('N');
  }
}

void loop() {
  if (digitalRead(A1) == HIGH) {
    pressed = false;
  } else {
    pressed = true;
  }
//  delay(10);
}
