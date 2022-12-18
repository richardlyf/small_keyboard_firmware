#include <Keyboard.h>
#include <limits.h>
#include <Wire.h>

#include "utils.h"

// command received from master
char cmd = '0';
// indicating new command was received
bool received = false;
// calibration initialized
bool initialized = false;
// calibration result finished
bool resultReady = false;
// keep track of which row of pin values have been sent
int pinIdx = 0;

int leftKeysInitValues[PCB::numMultiplexorReadPins][PCB::numMultiplexors];
int leftKeysPressedValues[PCB::numMultiplexorReadPins][PCB::numMultiplexors];
int leftKeysMaxMinValues[PCB::numMultiplexorReadPins][PCB::numMultiplexors][2]; // max, min

void receiveCommand(int howMany) {
  if (howMany != 1) {
    return;
  }

  while (Wire.available()) {
    cmd = Wire.read();
  }
  received = true;
}

void sendKeyValues() {
  if (!resultReady) {
    return;
  }
  // write 0 byte telling master the data is valid
  Wire.write(0);
  // calibration data is sent one row at a time as strings, not exceeding 32 bytes
  String data = "";
  for (byte i = 0; i < PCB::numMultiplexors; i++) {
    data += String(leftKeysInitValues[pinIdx][i]) + "," + String(leftKeysPressedValues[pinIdx][i]) + ",";
  }
  Serial.println(data);
  int dataLen = data.length() + 1;
  char data_array[dataLen];
  data.toCharArray(data_array, dataLen);
  Wire.write(data_array);
  pinIdx += 1;
}

void setup() {
  for (byte i = 0; i < PCB::numSelectorPins; i++) {
    pinMode(PCB::multiplexorSelectorPins[i], OUTPUT);
  }

  Wire.begin(SLAVE_ADDR);
  Wire.setClock(400000);
  // do not use internal pullups enabled by Wire.begin()
  digitalWrite(SDA, LOW);
  digitalWrite(SCL, LOW);

  Wire.onReceive(receiveCommand);
  Wire.onRequest(sendKeyValues);
  // Serial must begin after Wire, or neither works
  Serial.begin(9600);
  
  for (byte i = 0; i < PCB::numMultiplexorReadPins; i++) {
    for (byte j = 0; j < PCB::numMultiplexors; j++) {
      // set min values to int max
      leftKeysMaxMinValues[i][j][1] = INT_MAX;
    }
  }
}


void loop() {
  // record all keys initial positions and start calibration
  if (received && cmd == '1') {
    for (byte pin = 0; pin < PCB::numMultiplexorReadPins; pin++) {
      int sensorValues[PCB::numMultiplexors];
      readFromMultiplexors(sensorValues, pin);
      for (byte i = 0; i < PCB::numMultiplexors; i++) {
        const int sensorValue = sensorValues[i];
        leftKeysInitValues[pin][i] = sensorValue;
      }
    }
    received = false;
    initialized = true;
    resultReady = false;
    pinIdx = 0;
    Serial.println("Calibrating... start pressing keys. Receive 2 from Wire when done.");
  }

  // record max and min key values detected
  if (initialized) {
    // process left side of keyboard
    for (byte pin = 0; pin < PCB::numMultiplexorReadPins; pin++) {
      int sensorValues[PCB::numMultiplexors];
      readFromMultiplexors(sensorValues, pin);
      for (byte i = 0; i < PCB::numMultiplexors; i++) {
        const int sensorValue = sensorValues[i];
        leftKeysMaxMinValues[pin][i][0] = max(leftKeysMaxMinValues[pin][i][0], sensorValue);
        leftKeysMaxMinValues[pin][i][1] = min(leftKeysMaxMinValues[pin][i][1], sensorValue);
      }
    }
  }

  // stop calibration and output results
  if (received && cmd == '2') {
    initialized = false;
    for (byte pin = 0; pin < PCB::numMultiplexorReadPins; pin++) {
      for (byte i = 0; i < PCB::numMultiplexors; i++) {
        const int initValue = leftKeysInitValues[pin][i];
        const int maxValue = leftKeysMaxMinValues[pin][i][0];
        const int minValue = leftKeysMaxMinValues[pin][i][1];
        const int maxDiff = abs(maxValue - initValue);
        const int minDiff = abs(minValue - initValue);
        if (maxDiff > minDiff) {
          leftKeysPressedValues[pin][i] = maxValue;
        } else {
          leftKeysPressedValues[pin][i] = minValue;
        }
      }
    }

    resultReady = true;
  }
}
