#include <EEPROM.h>
#include <Wire.h>

#include "config.h"
#include "keyboardLED.hpp"
#include "smallKeyboard.hpp"
#include "utils.h"

State state;
// Keyboard wrapper that maintains state
SmallKeyboard smallKeyboard;
LED<RightLED> led(smallKeyboard);

void setup() {
  for (byte i = 0; i < PCB::numSelectorPins; i++) {
    pinMode(PCB::multiplexorSelectorPins[i], OUTPUT);
  }
  smallKeyboard.begin();
  Wire.begin();
  Wire.setClock(400000);

  // do not use internal pullups enabled by Wire.begin()
  digitalWrite(SDA, LOW);
  digitalWrite(SCL, LOW);
  // Serial must begin after Wire, or neither works
  Serial.begin(9600);
  state = NORMAL;
  // get key limit from EEPROM
  uint16_t lastRecordedKeyPress;
  EEPROM.get(0, lastRecordedKeyPress);
  smallKeyboard.setKeyPress(lastRecordedKeyPress);
}

void setKeyByCondition(String side, byte pin, byte multiplexorIdx, bool pressed) {
  if (pressed) {
    smallKeyboard.press(side, pin, multiplexorIdx);
  } else {
    smallKeyboard.release(side, pin, multiplexorIdx);
  }
}

void processRightKeyboard() {
  for (byte pin = 0; pin < PCB::numMultiplexorReadPins; pin++) {
    int sensorValues[PCB::numMultiplexors];
    readFromMultiplexors(sensorValues, pin);
    for (byte i = 0; i < PCB::numMultiplexors; i++) {
      const int key = PCB::rightMultiplexorPin2Char[pin][i];
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
      setKeyByCondition("right", pin, i, keyPressed);
    }
  }
}

void processLeftKeyboard() {
  bool receivedData = true;
  Wire.requestFrom(SLAVE_ADDR, PCB::i2cByteArraySize + 1);
  // check the first byte to see i2c did send valid data
  char validByte = Wire.read();
  if (validByte == -1) {
    receivedData = false;
  }
  if (!receivedData) {
    return;
  }

  for (byte i = 0; i <PCB::i2cByteArraySize; i++) {
    const byte dataByte = Wire.read();
    // skip over any extra unpopulated bytes at the end
    if (dataByte == -1) {
      continue;
    }

    for (byte bitIndex = 0; bitIndex < BYTE_SIZE; bitIndex++) {
      const byte globalBitIndex = i * BYTE_SIZE + bitIndex;
      const int pin = globalBitIndex / PCB::numMultiplexors;
      const int multiplexorIdx = globalBitIndex % PCB::numMultiplexors;
      const int key = PCB::leftMultiplexorPin2Char[pin][multiplexorIdx];
      if (key == KEY_EMPTY) {
        continue;
      }
      const bool keyPressed = bitRead(dataByte, bitIndex);
      setKeyByCondition("left", pin, multiplexorIdx, keyPressed);
    }
  }
}

void checkStateTransition() {
  // check macro for state change
  // Macro 0 starts the keyboard as normal
  if (smallKeyboard.isKeyPressed("left", 6, 0)){
    state = NORMAL;
    led.setState(NORMAL);
    led.sendSignalToLeft(NORMAL);
    smallKeyboard.resetKeyPress();
    EEPROM.put(0, smallKeyboard.numKeyPress());
  }
  // Macro 1 enters debug mode
  else if (smallKeyboard.isKeyPressed("left", 5, 0)){
    state = DEBUG;
    led.setState(NORMAL);
    led.sendSignalToLeft(NORMAL);
  }
  // Macro 2 & 3 kills the keyboard
  else if (smallKeyboard.isKeyPressed("left", 4, 0) && smallKeyboard.isKeyPressed("left", 3, 0)) {
    state = KILLED;
    led.setState(KILLED);
    led.sendSignalToLeft(KILLED);
  }
  // check key press limits
  if (smallKeyboard.numKeyPress() > MAX_KEY_LIMIT) {
    state = PAUSED;
    led.setState(PAUSED);
    led.sendSignalToLeft(PAUSED);
  }
}

void loop() {
  switch (state) {
    case PAUSED:
      // fall through to debug
    case DEBUG:
      smallKeyboard.setDebug(true);
      break;
    case NORMAL:
      smallKeyboard.setDebug(false);
      break;
    case KILLED:
      return;
  }
  processRightKeyboard();
  processLeftKeyboard();
  EEPROM.put(0, smallKeyboard.numKeyPress());
  checkStateTransition();

  // LED
  led.update();
  led.flushToLED();
}
