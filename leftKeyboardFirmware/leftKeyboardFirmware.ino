#include <Wire.h>
#include <Keyboard.h>

#include "config.h"

byte keyPressArray[PCB::i2cByteArraySize];

void setup() {
  for (byte i = 0; i < PCB::numSelectorPins; i++) {
    pinMode(PCB::multiplexorSelectorPins[i], OUTPUT);
  }
  // Keyboard.begin();

  // left keyboard is the slave
  Wire.begin(SLAVE_ADDR);
  Wire.setClock(400000);

  digitalWrite(SDA, LOW);
  digitalWrite(SCL, LOW);
  Wire.onRequest(sendAllKeyPressViaI2C);
  // Serial must begin after Wire, or neither works
  Serial.begin(9600);
}

void sendAllKeyPressViaI2C() {
  Wire.write(0);
  Wire.write(keyPressArray, PCB::i2cByteArraySize);
}

void loop() {  
  for (byte pin = 0; pin < PCB::numMultiplexorReadPins; pin++) {
    int sensorValues[PCB::numMultiplexors];
    readFromMultiplexors(sensorValues, pin);
    for (byte i = 0; i < PCB::numMultiplexors; i++) {
      const char key = PCB::leftMultiplexorPin2Char[pin][i];
      const int keyValue = sensorValues[i];

      // skip empty keys
      if (key == KEY_EMPTY) {
        continue;
      }
      
      // don't do anything with function keys for now
      if (key == KEY_FN) {
        continue;
      }

      // check if key is pressed
      const int unpressedValue = config::leftKeysThresholds[pin][i][0];
      const int pressedValue = config::leftKeysThresholds[pin][i][1];
      const int keyPressRange = pressedValue - unpressedValue;
      // key is not calibrated, skip
      if (abs(keyPressRange) < MAX_KEY_NOISE) {
        continue;
      }
      const int keyThreshold = unpressedValue + keyPressRange * KEY_THRESHOLD_PERCENTAGE;
      const bool keyPressed = ((keyPressRange > 0 && keyValue > keyThreshold) || (keyPressRange < 0 && keyValue < keyThreshold));
      if (keyPressed) {
        Serial.println(String(key) + " pressed!");
      }
      // write key press result to key press array
      const byte globalBitIndex = pin * PCB::numMultiplexors + i;
      const int byteArrayIndex = globalBitIndex / BYTE_SIZE;
      const int bitIndex = globalBitIndex % BYTE_SIZE;
      bitWrite(keyPressArray[byteArrayIndex], bitIndex, keyPressed);
    }
  }
}
