#ifndef _UTILS_H
#define _UTILS_H

#define SLAVE_ADDR 4  // random slave address
#define BYTE_SIZE 8	  // 8 bits in a byte

#define KEY_EMPTY 0x0
// function key doesn't have a scan code itself but modifies other keys' scan codes
#define KEY_FN 0xf
// macro keys
#define MACRO_0 0xa0
#define MACRO_1 0xa1
#define MACRO_2 0xa2
#define MACRO_3 0xa3
#define MACRO_4 0xa4
#define KEY_PRINT_SCREEN 0xCE

#define KEY_LEFT_SPACE ' '
#define KEY_RIGHT_SPACE ' '

namespace PCB {
    const int numMultiplexors = 3;
    const int multiplexorId2DataPin[numMultiplexors] = {A1, A0, A2};
    const int numSelectorPins = 4;
    const int multiplexorSelectorPins[numSelectorPins] = {4, 7, 8, 12};
    const int numMultiplexorReadPins = 16;

    // number of bytes needed to represent each key as a single bit
    const int i2cByteArraySize = (numMultiplexorReadPins * numMultiplexors + BYTE_SIZE - 1) / BYTE_SIZE;

    // left keyboard setup
    const int leftMultiplexorPin2Char[numMultiplexorReadPins][numMultiplexors] = {
        {KEY_LEFT_CTRL,     KEY_LEFT_ALT,       KEY_EMPTY},
        {KEY_FN,            'x',                'c'},
        {MACRO_4,           KEY_EMPTY,          'f'},
        {MACRO_3,           KEY_EMPTY,          'r'},
        {MACRO_2,           KEY_F4,             't'},
        {MACRO_1,           KEY_F3,             '4'},
        {MACRO_0,           '2',                '5'},
        {KEY_ESC,           KEY_F2,             KEY_F5},
        {'1',               'q',                KEY_F6},
        {KEY_F1,            '3',                '6'},
        {'`',               'w',                KEY_EMPTY},
        {KEY_TAB,           'e',                KEY_EMPTY},
        {KEY_LEFT_GUI,      'd',                'g'},
        {KEY_LEFT_SHIFT,    'z',                'v'},
        {KEY_EMPTY,         's',                KEY_LEFT_SPACE},
        {KEY_CAPS_LOCK,     'a',                'b'}
    };

    // right keyboard setup
    const int rightMultiplexorPin2Char[numMultiplexorReadPins][numMultiplexors] = {
        {KEY_LEFT_ARROW,  'o',                KEY_RIGHT_SPACE},
        {KEY_RIGHT_SHIFT, 'i',                'n'},
        {KEY_RIGHT_CTRL,  '0',                'm'},
        {'/',             '-',                KEY_RIGHT_ALT},
        {'.',             '=',                'y'},
        {'l',             KEY_F10,            KEY_F8},
        {';',             KEY_F11,            KEY_F7},
        {'"',             KEY_F12,            '7'},
        {KEY_END,         KEY_DELETE,         'u'},
        {'\\',            KEY_INSERT,         '8'},
        {KEY_RETURN,      KEY_PRINT_SCREEN,   KEY_F9},
        {KEY_PAGE_UP,     KEY_BACKSPACE,      '9'},
        {KEY_PAGE_DOWN,   KEY_HOME,           'k'},
        {KEY_UP_ARROW,    'p',                'j'},
        {KEY_RIGHT_ARROW, '[',                ','},
        {KEY_DOWN_ARROW,  ']',                'h'}
    };
}

// helper functions

// read sensor values from all three multiplexors for the given pin
void readFromMultiplexors(int* sensorValues, byte pin) {
  for (byte i = 0; i < PCB::numSelectorPins; i++) {
    const bool state = bitRead(pin, i);
    digitalWrite(PCB::multiplexorSelectorPins[i], state);
  }
  for (byte i = 0; i < PCB::numMultiplexors; i++) {
    sensorValues[i] = analogRead(PCB::multiplexorId2DataPin[i]);
  }
}

#endif // _UTILS_H