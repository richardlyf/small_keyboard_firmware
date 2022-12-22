#include <Wire.h>

#include "config.h"
#include "keyboardLED.hpp"

byte keyPressArray[PCB::i2cByteArraySize];
// even tho right keyboard does all the processing, we create a keyboard object
// for the left keyboard just so it could reduce compute load of the right side
// and keep track of left keyboard state separately
SmallKeyboard smallKeyboard;
LED<LeftLED> led(smallKeyboard);

void setup() {
  for (byte i = 0; i < PCB::numSelectorPins; i++) {
    pinMode(PCB::multiplexorSelectorPins[i], OUTPUT);
  }

  // left keyboard is the slave
  Wire.begin(SLAVE_ADDR);
  Wire.setClock(400000);

  digitalWrite(SDA, LOW);
  digitalWrite(SCL, LOW);
  Wire.onRequest(sendAllKeyPressViaI2C);
  Wire.onReceive(updateLED);
  // Serial must begin after Wire, or neither works
  Serial.begin(9600);
  smallKeyboard.setDebug(true);
}

void sendAllKeyPressViaI2C() {
  Wire.write(0);
  Wire.write(keyPressArray, PCB::i2cByteArraySize);
}

void updateLED(int bytesRead) {
  if (Wire.available()) {
    byte state = Wire.read();
    led.setState(state);
  }
}

void setKeyByCondition(String side, byte pin, byte multiplexorIdx, bool pressed) {
  if (pressed) {
    smallKeyboard.press(side, pin, multiplexorIdx);
  } else {
    smallKeyboard.release(side, pin, multiplexorIdx);
  }
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
      setKeyByCondition("left", pin, i, keyPressed);
      // write key press result to key press array
      const byte globalBitIndex = pin * PCB::numMultiplexors + i;
      const int byteArrayIndex = globalBitIndex / BYTE_SIZE;
      const int bitIndex = globalBitIndex % BYTE_SIZE;
      bitWrite(keyPressArray[byteArrayIndex], bitIndex, keyPressed);
    }
  }

  led.update();
  led.renderMacros();
  led.flushToLED();
}
