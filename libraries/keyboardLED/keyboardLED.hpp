#ifndef _KEYBOARD_LED_HPP
#define _KEYBOARD_LED_HPP

#include <Wire.h>
#include <PololuLedStrip.h>

#include "utils.h"
#include "smallKeyboard.hpp"

struct LEDIdx {
    byte rowIdx;
    byte colIdx;
};

class LeftLED {
    public:
        // left side
        PololuLedStrip<13> L_LED0;
        PololuLedStrip<11> L_LED1;
        PololuLedStrip<10>  L_LED2;
        PololuLedStrip<9>  L_LED3;
        PololuLedStrip<6>  L_LED4;
        PololuLedStrip<5> L_LED5;
        PololuLedStripBase* LEDs[6] = {&L_LED0, &L_LED1, &L_LED2, &L_LED3, &L_LED4, &L_LED5};

        rgb_color L_L0Val[8];
        rgb_color L_L1Val[8];
        rgb_color L_L2Val[7];
        rgb_color L_L3Val[7];
        rgb_color L_L4Val[7];
        rgb_color L_L5Val[5];
        rgb_color* LEDVals[6] = {L_L0Val, L_L1Val, L_L2Val, L_L3Val, L_L4Val, L_L5Val};

        const LEDIdx keyToIdx[PCB::numMultiplexorReadPins][PCB::numMultiplexors] = {
            {{5, 1},                {5, 3},                   {KEY_EMPTY, KEY_EMPTY}},
            {{5, 0},                {4, 3},                   {4, 4}},
            {{4, 0},                {KEY_EMPTY, KEY_EMPTY},   {3, 5}},
            {{3, 0},                {KEY_EMPTY, KEY_EMPTY},   {2, 5}},
            {{2, 0},                {0, 5},                   {2, 6}},
            {{1, 0},                {0, 4},                   {1, 5}},
            {{0, 0},                {1, 3},                   {1, 6}},
            {{0, 1},                {0, 3},                   {0, 6}},
            {{1, 2},                {2, 2},                   {0, 7}},
            {{0, 2},                {1, 4},                   {1, 7}},
            {{1, 1},                {2, 3},                   {KEY_EMPTY, KEY_EMPTY}},
            {{2, 1},                {2, 4},                   {KEY_EMPTY, KEY_EMPTY}},
            {{5, 2},                {3, 4},                   {3, 6}},
            {{4, 1},                {4, 2},                   {4, 5}},
            {{KEY_EMPTY, KEY_EMPTY},{3, 3},                   {5, 4}},
            {{3, 1},                {3, 2},                   {4, 6}},
        };

        int* pinToChar = &PCB::leftMultiplexorPin2Char[0][0];
};

class RightLED{
    public:
        // right side
        PololuLedStrip<11> R_LED0;
        PololuLedStrip<10> R_LED1;
        PololuLedStrip<9>  R_LED2;
        PololuLedStrip<6>  R_LED3;
        PololuLedStrip<5>  R_LED4;
        PololuLedStrip<13> R_LED5;
        PololuLedStripBase* LEDs[6] = {&R_LED0, &R_LED1, &R_LED2, &R_LED3, &R_LED4, &R_LED5};

        rgb_color R_L0Val[9];
        rgb_color R_L1Val[8];
        rgb_color R_L2Val[9];
        rgb_color R_L3Val[8];
        rgb_color R_L4Val[8];
        rgb_color R_L5Val[6];
        rgb_color* LEDVals[6] = {R_L0Val, R_L1Val, R_L2Val, R_L3Val, R_L4Val, R_L5Val};

        const LEDIdx keyToIdx[PCB::numMultiplexorReadPins][PCB::numMultiplexors] = {
            {{5, 2}, {2, 5}, {5, 5}},
            {{4, 2}, {2, 6}, {4, 7}},
            {{5, 3}, {1, 4}, {4, 6}},
            {{4, 3}, {1, 3}, {5, 4}},
            {{4, 4}, {1, 2}, {2, 8}},
            {{3, 4}, {0, 5}, {0, 7}},
            {{3, 3}, {0, 4}, {0, 8}},
            {{3, 2}, {0, 3}, {1, 7}},
            {{2, 0}, {0, 0}, {2, 7}},
            {{2, 1}, {0, 1}, {1, 6}},
            {{3, 1}, {0, 2}, {0, 6}},
            {{3, 0}, {1, 1}, {1, 5}},
            {{4, 0}, {1, 0}, {3, 5}},
            {{4, 1}, {2, 4}, {3, 6}},
            {{5, 0}, {2, 3}, {4, 5}},
            {{5, 1}, {2, 2}, {3, 7}},
        };

        int* pinToChar = &PCB::rightMultiplexorPin2Char[0][0];

};

template<typename BaseLED>
class LED {
    public:
        LED (const SmallKeyboard& keyboard): _keyboard{keyboard} {}
        void flushToLED();
        void writeLED(byte pin, byte multiplexorIdx, rgb_color color);
        bool* getKeyStateArray();

        void update();
        // only use for left side
        void renderMacros();
        // only use for right side
        void sendSignalToLeft(State state);
        void setState(State state) { _ledState = state; }

    private:
        const SmallKeyboard& _keyboard;
        BaseLED _led;
        State _ledState = NORMAL;

        void greenWhenPressed();
        void setAllToColor(const rgb_color& color);
};


// Implementations since we can't use cpp file for class template

template<typename BaseLED>
void LED<BaseLED>::flushToLED() {
    for (byte ledID = 0; ledID < 6; ledID++) {
        _led.LEDs[ledID]->write(_led.LEDVals[ledID], 10);
    }
}

template<typename BaseLED>
void LED<BaseLED>::writeLED(byte pin, byte multiplexorIdx, rgb_color color) {
    LEDIdx ledIdx = _led.keyToIdx[pin][multiplexorIdx];
    rgb_color& colorStored = _led.LEDVals[ledIdx.rowIdx][ledIdx.colIdx];
    colorStored = color;
}

template<>
bool* LED<LeftLED>::getKeyStateArray() {
    return &_keyboard._leftKeyState[0][0];
}

template<>
bool* LED<RightLED>::getKeyStateArray() {
    return &_keyboard._rightKeyState[0][0];
}

template<typename BaseLED>
void LED<BaseLED>::setAllToColor(const rgb_color& color) {
    for (byte i = 0; i < PCB::numMultiplexorReadPins; i++) {
        for (byte j = 0; j < PCB::numMultiplexors; j++) {
            writeLED(i, j, color);
        }
    }
}

template<typename BaseLED>
void LED<BaseLED>::greenWhenPressed() {
    // light up pressed keys green
    bool* keyState = getKeyStateArray();
    for (byte pin = 0; pin < PCB::numMultiplexorReadPins; pin++) {
        for (byte multiplexorIdx = 0; multiplexorIdx < PCB::numMultiplexors; multiplexorIdx++) {
            int arrayIdx = pin * PCB::numMultiplexors + multiplexorIdx;
            int key = _led.pinToChar[arrayIdx];
            if (key == KEY_EMPTY) {
                continue;
            }
            if (keyState[arrayIdx]) {
                writeLED(pin, multiplexorIdx, {0, 128, 0});
            } else {
                writeLED(pin, multiplexorIdx, {0, 0, 0});
            }
        }
    }
}

template<typename BaseLED>
void LED<BaseLED>::update() {
    switch (_ledState) {
        case PAUSED:
            setAllToColor({128, 0, 0});
            return;
        case KILLED:
            setAllToColor({0, 0, 0});
            return;
        default:
            greenWhenPressed();
    }
}

template<>
void LED<LeftLED>::renderMacros() {
    // Macro 0 green
    writeLED(6, 0, {0, 128, 0});
    // Macro 1 yellow
    writeLED(5, 0, {128, 128, 0});
    // Macro 2 & 3 red when not killed, otherwise purple
    if (_ledState != KILLED) {
        writeLED(4, 0, {128, 0, 0});
        writeLED(3, 0, {128, 0, 0});
    } else {
        writeLED(4, 0, {148, 0, 211});
        writeLED(3, 0, {148, 0, 211});
    }
}

template<>
void LED<RightLED>::sendSignalToLeft(State state) {
    Wire.beginTransmission(SLAVE_ADDR);
    Wire.write(state);
    Wire.endTransmission();
}

#endif // _KEYBOARD_LED_HPP

