#include <Wire.h>

void setup() {
  // put your setup code here, to run once:
  Wire.begin();
  digitalWrite(SDA, LOW);
  digitalWrite(SCL, LOW);
  Wire.setClock(400000);
  pinMode(13, OUTPUT);
  Serial.begin(9600);
}

void loop() {
  // put your main code here, to run repeatedly:
//   digitalWrite(13, HIGH);
  int num = Wire.requestFrom(4, 1);
//  Serial.println("test");
  if (Wire.available()) {
     byte data = Wire.read();
     if (data == 89) digitalWrite(13, HIGH);
     else digitalWrite(13, LOW);
     Serial.println(data);
  }
//  delay(10);
}
