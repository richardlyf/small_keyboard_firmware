#include <Keyboard.h>
#include <PololuLedStrip.h>
#include <Wire.h>

#include "config.h"
#include "utils.h"

PololuLedStrip<11> LED0;
PololuLedStrip<10> LED1;
PololuLedStrip<9> LED2;
PololuLedStrip<6> LED3;
PololuLedStrip<5> LED4;
PololuLedStrip<13> LED5;

void setup() {
  for (byte i = 0; i < PCB::numSelectorPins; i++) {
    pinMode(PCB::multiplexorSelectorPins[i], OUTPUT);
  }
//  Keyboard.begin();
  Wire.begin();
  Wire.setClock(400000);

  // do not use internal pullups enabled by Wire.begin()
  digitalWrite(SDA, LOW);
  digitalWrite(SCL, LOW);
  // Serial must begin after Wire, or neither works
  Serial.begin(9600);
}

//void setKeyByCondition(int key, bool pressed) {
//  if (pressed) {
//    Keyboard.press(key);
//  } else {
//    Keyboard.release(key);
//  }
//}

void processRightKeyboard() {
 for (byte pin = 0; pin < PCB::numMultiplexorReadPins; pin++) {
    int sensorValues[PCB::numMultiplexors];
    readFromMultiplexors(sensorValues, pin);
    for (byte i = 0; i < PCB::numMultiplexors; i++) {
      const char key = PCB::rightMultiplexorPin2Char[pin][i];
      const int keyValue = sensorValues[i];

      // check if key is pressed
      const int unpressedValue = config::rightKeysThresholds[pin][i][0];
      const int pressedValue = config::rightKeysThresholds[pin][i][1];
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
      // if (pin == 15 && i == 0 && sensorValues[i] < 470){
      //   colors[1] = {255, 0, 0};
      //   Serial.println("down");
      // }
      //   if (pin == 14 && i == 0 && sensorValues[i] > 544){
      //   colors[0] = {255, 0, 0};
      //   Serial.println("right");
      // }


//      setKeyByCondition(key, keyPressed);
    }
  }
}

void loop() {
  rgb_color colors[3];
  for(int i = 0; i < 3; i++) {
    colors[i] = {0,0,0};
  }
  
  // process right side of keyboard
  processRightKeyboard();

  // process left side of keyboard
  bool receivedData = true;
  Wire.requestFrom(SLAVE_ADDR, PCB::i2cByteArraySize + 1);
  char validByte = Wire.read();
  if (validByte == -1) {
    receivedData = false;
  }
  if (receivedData) {
    for (byte i = 0; i <PCB::i2cByteArraySize; i++) {
      const byte dataByte = Wire.read();
      if (dataByte == -1) {
        continue;
      }
      
      for (byte bitIndex = 0; bitIndex < BYTE_SIZE; bitIndex++) {
        const byte globalBitIndex = i * BYTE_SIZE + bitIndex;
        const int pin = globalBitIndex / PCB::numMultiplexors;
        const int multiplexorIdx = globalBitIndex % PCB::numMultiplexors;
        const char key = PCB::leftMultiplexorPin2Char[pin][multiplexorIdx];
        if (key == KEY_EMPTY) {
          continue;
        }
        const bool keyPressed = bitRead(dataByte, bitIndex);
    //      setKeyByCondition(key, keyPressed);
        if (keyPressed) {
          Serial.println(String(key) + " pressed!");
        }
      }
    }
  }

  // LED
  LED5.write(colors, 3);
}
