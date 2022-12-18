#include <Keyboard.h>
#include <limits.h>
#include <Wire.h>

#include "utils.h"

bool initialized = false;
int rightKeysInitValues[PCB::numMultiplexorReadPins][PCB::numMultiplexors];
int rightKeysPressedValues[PCB::numMultiplexorReadPins][PCB::numMultiplexors];
int rightKeysMaxMinValues[PCB::numMultiplexorReadPins][PCB::numMultiplexors][2]; // max, min

void printKeyArray(int keyArray[PCB::numMultiplexorReadPins][PCB::numMultiplexors]) {
  String out = "";
  for (byte pin = 0; pin < PCB::numMultiplexorReadPins; pin++) {
    out += "{";
    for (byte i = 0; i < PCB::numMultiplexors; i++) {
      out += String(keyArray[pin][i]) + ",";
    }
    out += "}, \n";
  }
  Serial.println(out);
}

void printRightCalibrationResult(int initValues[PCB::numMultiplexorReadPins][PCB::numMultiplexors],
                            int pressedValues[PCB::numMultiplexorReadPins][PCB::numMultiplexors]) {
  String out = "";
  for (byte pin = 0; pin < PCB::numMultiplexorReadPins; pin++) {
    out += "{";
    for (byte i = 0; i < PCB::numMultiplexors; i++) {
      out += "{" + String(initValues[pin][i]) + "," + String(pressedValues[pin][i]) + "}, ";
    }
    out += "}, \n";
  }
  Serial.println(out);
}

void printLeftCalibrationResult() {
  String out = "";

  for (byte pin = 0; pin < PCB::numMultiplexorReadPins;) {
    Wire.requestFrom(SLAVE_ADDR, 32);
    char validByte = Wire.read();
    if (validByte == -1) {
      // no data returned, wait for left keyboard calibration to finish  
      Serial.println("waiting");
      continue;
    }
    pin += 1;
    
    // read entire line from i2c
    String line = "";
    while (Wire.available()) {
      char c = Wire.read();
      // skip any invalid bytes at the end of the line
      if (c == -1) {
        break;
      }
      line += c;
    }

    // format line
    out += "{";
    int startIdx = 0;
    for (int i = 0; i < PCB::numMultiplexors; i++) {
      // we are looking for every other comma
      int firstIdx = line.indexOf(',', startIdx);
      int secondIdx = line.indexOf(',', firstIdx + 1);
      out += "{" + line.substring(startIdx, secondIdx) + "}, ";
      startIdx = secondIdx + 1;
    }
    out += "}, \n";
  }

  Serial.println(out);
}

void setup() {
  for (byte i = 0; i < PCB::numSelectorPins; i++) {
    pinMode(PCB::multiplexorSelectorPins[i], OUTPUT);
  }

  Wire.begin();
  Wire.setClock(400000);
  // do not use internal pullups enabled by Wire.begin()
  digitalWrite(SDA, LOW);
  digitalWrite(SCL, LOW);
  // Serial must begin after Wire, or neither works
  Serial.begin(9600);
  
  for (byte i = 0; i < PCB::numMultiplexorReadPins; i++) {
    for (byte j = 0; j < PCB::numMultiplexors; j++) {
      // set min values to int max
      rightKeysMaxMinValues[i][j][1] = INT_MAX;
    }
  }
  Serial.println("Enter 1 in serial to start calibration. Make sure no keys are pressed!");
}


void loop() {
  // enter 1 to record all keys initial positions and start calibration
  if (Serial.available() > 0 && Serial.read() == '1') {
    // send same signal to left keyboard
    Wire.beginTransmission(SLAVE_ADDR);
    Wire.write('1');
    Wire.endTransmission();
    // get init values for right keyboard
    for (byte pin = 0; pin < PCB::numMultiplexorReadPins; pin++) {
      int sensorValues[PCB::numMultiplexors];
      readFromMultiplexors(sensorValues, pin);
      for (byte i = 0; i < PCB::numMultiplexors; i++) {
        const int sensorValue = sensorValues[i];
        rightKeysInitValues[pin][i] = sensorValue;
      }
    }
    initialized = true;
    Serial.println("Calibrating... start pressing keys. Enter 2 in serial when done.");
  }

  // record max and min key values detected
  if (initialized) {
    // process right side of keyboard
    for (byte pin = 0; pin < PCB::numMultiplexorReadPins; pin++) {
      int sensorValues[PCB::numMultiplexors];
      readFromMultiplexors(sensorValues, pin);
      for (byte i = 0; i < PCB::numMultiplexors; i++) {
        const int sensorValue = sensorValues[i];
        rightKeysMaxMinValues[pin][i][0] = max(rightKeysMaxMinValues[pin][i][0], sensorValue);
        rightKeysMaxMinValues[pin][i][1] = min(rightKeysMaxMinValues[pin][i][1], sensorValue);
      }
    }
  }

  // enter 2 to stop calibration and output results
  if (Serial.available() > 0 && Serial.read() == '2') {
    initialized = false;
    // send same signal to left keyboard
    Wire.beginTransmission(SLAVE_ADDR);
    Wire.write('2');
    Wire.endTransmission();
    // compile calibration results for right keyboard
    for (byte pin = 0; pin < PCB::numMultiplexorReadPins; pin++) {
      for (byte i = 0; i < PCB::numMultiplexors; i++) {
        const int initValue = rightKeysInitValues[pin][i];
        const int maxValue = rightKeysMaxMinValues[pin][i][0];
        const int minValue = rightKeysMaxMinValues[pin][i][1];
        const int maxDiff = abs(maxValue - initValue);
        const int minDiff = abs(minValue - initValue);
        if (maxDiff > minDiff) {
          rightKeysPressedValues[pin][i] = maxValue;
        } else {
          rightKeysPressedValues[pin][i] = minValue;
        }
      }
    }
    Serial.println("Right keyboard calibration result:");
    printRightCalibrationResult(rightKeysInitValues, rightKeysPressedValues);
    Serial.println();
    Serial.println("Left keyboard calibration result:");
    printLeftCalibrationResult();
  }
}
