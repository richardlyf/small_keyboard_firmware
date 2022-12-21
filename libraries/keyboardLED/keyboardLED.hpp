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

class BaseLED {
  public:
    virtual void update(String side);
    void flushToLED(String side);
    void writeLED(String side, byte pin, byte multiplexorIdx, rgb_color color);

  private:
    // left side
    PololuLedStrip<13> L_LED0;
    PololuLedStrip<11> L_LED1;
    PololuLedStrip<10>  L_LED2;
    PololuLedStrip<9>  L_LED3;
    PololuLedStrip<6>  L_LED4;
    PololuLedStrip<5> L_LED5;
    PololuLedStripBase* leftLEDs[6] = {&L_LED0, &L_LED1, &L_LED2, &L_LED3, &L_LED4, &L_LED5};

    rgb_color L_L0Val[8];
    rgb_color L_L1Val[8];
    rgb_color L_L2Val[7];
    rgb_color L_L3Val[7];
    rgb_color L_L4Val[7];
    rgb_color L_L5Val[5];
    rgb_color* leftLEDVals[6] = {L_L0Val, L_L1Val, L_L2Val, L_L3Val, L_L4Val, L_L5Val};

    const LEDIdx leftKeyToIdx[PCB::numMultiplexorReadPins][PCB::numMultiplexors] = {
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

    // right side
    PololuLedStrip<11> R_LED0;
    PololuLedStrip<10> R_LED1;
    PololuLedStrip<9>  R_LED2;
    PololuLedStrip<6>  R_LED3;
    PololuLedStrip<5>  R_LED4;
    PololuLedStrip<13> R_LED5;
    PololuLedStripBase* rightLEDs[6] = {&R_LED0, &R_LED1, &R_LED2, &R_LED3, &R_LED4, &R_LED5};

    rgb_color R_L0Val[9];
    rgb_color R_L1Val[8];
    rgb_color R_L2Val[7];
    rgb_color R_L3Val[8];
    rgb_color R_L4Val[8];
    rgb_color R_L5Val[6];
    rgb_color* rightLEDVals[6] = {R_L0Val, R_L1Val, R_L2Val, R_L3Val, R_L4Val, R_L5Val};

    const LEDIdx rightKeyToIdx[PCB::numMultiplexorReadPins][PCB::numMultiplexors] = {
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
};

class LED : public BaseLED {
    public:
        LED (const SmallKeyboard& keyboard): _keyboard{keyboard} {} 
        void update(String side);
        void sendSignalToLeft(bool state);
        void setTrigger(bool state) { _triggered = state; }

    private:
        SmallKeyboard _keyboard;
        bool _triggered = false;
};

#endif // _KEYBOARD_LED_HPP

