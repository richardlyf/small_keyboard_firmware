#include "keyboardLED.hpp"

void BaseLED::flushToLED(String side) {
    if (side == "left") {
        for (byte ledID = 0; ledID < 6; ledID++) {
            leftLEDs[ledID]->write(leftLEDVals[ledID], 10);
        }    
    } else if (side == "right") {
        for (byte ledID = 0; ledID < 6; ledID++) {
            rightLEDs[ledID]->write(rightLEDVals[ledID], 10);
        }
    }
}

void BaseLED::writeLED(String side, byte pin, byte multiplexorIdx, rgb_color color) {
    LEDIdx* keyToIdx;
    rgb_color** ledVals;
    if (side == "left") {
        keyToIdx = &leftKeyToIdx[0][0];
        ledVals = &leftLEDVals[0];
    } else if (side == "right") {
        keyToIdx = &rightKeyToIdx[0][0];
        ledVals = &rightLEDVals[0];
    }

    byte keyIdx = pin * PCB::numMultiplexors + multiplexorIdx;
    LEDIdx ledIdx = keyToIdx[keyIdx];
    rgb_color& colorStored = ledVals[ledIdx.rowIdx][ledIdx.colIdx];
    colorStored = color;
}

void LED::update(String side) {
    // if triggered, all red
    if (_triggered) {
        for (byte i = 0; i < PCB::numMultiplexorReadPins; i++) {
            for (byte j = 0; j < PCB::numMultiplexors; j++) {
                writeLED(side, i, j, {255, 0, 0});
            }
        }
        return;
    }
    
    // light up pressed keys green
    int* pinToChar;
    bool* keyState;
    if (side == "left") {
        pinToChar = &PCB::leftMultiplexorPin2Char[0][0];
        keyState = &_keyboard._leftKeyState[0][0];
    } else if (side == "right") {
        pinToChar = &PCB::rightMultiplexorPin2Char[0][0];
        keyState = &_keyboard._rightKeyState[0][0];
    }

    for (byte pin = 0; pin < PCB::numMultiplexorReadPins; pin++) {
        for (byte multiplexorIdx = 0; multiplexorIdx < PCB::numMultiplexors; multiplexorIdx++) {
            int arrayIdx = pin * PCB::numMultiplexors + multiplexorIdx;
            int key = pinToChar[arrayIdx];
            if (key == KEY_EMPTY) {
                return;
            }
            if (keyState[arrayIdx]) {
                // Serial.println("here  " + side);
                // Serial.println(arrayIdx);
    //             // Serial.println(char(key));
                // writeLED(side, pin, multiplexorIdx, {0, 255, 0});
            }
        }
    }
}

void LED::sendSignalToLeft(bool state) {
    Wire.beginTransmission(SLAVE_ADDR);
    Wire.write(state);
    Wire.endTransmission();
}

