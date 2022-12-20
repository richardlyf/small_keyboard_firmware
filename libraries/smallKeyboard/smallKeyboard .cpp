#include "smallKeyboard.hpp"

static String keySignalToChar(int key) {
    switch(key){
        case MACRO_0:
            return "MACRO_0";
        case MACRO_1:
            return "MACRO_1";
        case MACRO_2:
            return "MACRO_2";
        case MACRO_3:
            return "MACRO_3";
        case MACRO_4:
            return "MACRO_4";

        case KEY_ESC:
            return "ESC";
        case KEY_FN:
            return "FN";

        case KEY_F1:
            return "F1";
        case KEY_F2:
            return "F2";
        case KEY_F3:
            return "F3";
        case KEY_F4:
            return "F4";
        case KEY_F5:
            return "F5";
        case KEY_F6:
            return "F6";
        case KEY_F7:
            return "F7";
        case KEY_F8:
            return "F8";
        case KEY_F9:
            return "F9";
        case KEY_F10:
            return "F10";
        case KEY_F11:
            return "F11";
        case KEY_F12:
            return "F12";

        case KEY_UP_ARROW:
            return "UP_ARROW";
        case KEY_DOWN_ARROW:
            return "DOWN_ARROW";
        case KEY_LEFT_ARROW:
            return "LEFT_ARROW";
        case KEY_RIGHT_ARROW:
            return "RIGHT_ARROW";

        case KEY_LEFT_CTRL:
            return "L_CTRL";
        case KEY_TAB:
            return "TAB";
        case KEY_LEFT_GUI:
            return "GUI";
        case KEY_LEFT_SHIFT:
            return "L_SHIFT";
        case KEY_CAPS_LOCK:
            return "CAPs";
        case KEY_LEFT_ALT:
            return "L_ALT";
        case KEY_LEFT_SPACE:
            return "L_SPACE";

        case KEY_RIGHT_SHIFT:
            return "R_SHIFT";
        case KEY_RIGHT_CTRL:
            return "R_CTRL";
        case KEY_END:
            return "END";
        case KEY_RETURN:
            return "ENTER";
        case KEY_PAGE_UP:
            return "P_UP";
        case KEY_PAGE_DOWN:
            return "P_DN";
        case KEY_DELETE:
            return "DEL";
        case KEY_INSERT:
            return "INS";
        case KEY_PRINT_SCREEN:
            return "PRT SC";
        case KEY_BACKSPACE:
            return "BACKSPACE";
        case KEY_HOME:
            return "HOME";
        case KEY_RIGHT_SPACE:
            return "R_SPACE";
        case KEY_RIGHT_ALT:
            return "R_ALT";
    }
    return "";
}

bool SmallKeyboard::isMacro(int key) {
    if (key == KEY_FN) {
        return true;
    }
    for (byte i = 0; i < 5; i++) {
        if (key == MACRO_0 + i) {
            return true;
        }
    }
    return false;
}

void SmallKeyboard::processKey(String side, String action, byte pin, byte multiplexorIdx) {
    // create variables that either reference the left or right side of the keyboard
    // unfortunately this is the easiest way to reference an array without creating a class
    int* pinToChar;
    bool* keyState;
    if (side == "left") {
        pinToChar = &PCB::leftMultiplexorPin2Char[0][0];
        keyState = &_leftKeyState[0][0];
    } else if (side == "right") {
        pinToChar = &PCB::rightMultiplexorPin2Char[0][0];
        keyState = &_rightKeyState[0][0];
    }
    
    // ignore press if state is true; ignore release if state is false
    bool ignoreActionCondition;
    size_t (Keyboard_::*actionFn)(uint8_t);
    if (action == "press") {
        actionFn = &Keyboard_::press;
        ignoreActionCondition = true;
    } else if (action == "release") {
        actionFn = &Keyboard_::release;
        ignoreActionCondition = false;
    }

    // need to compute arrayIdx ourselves since 2d array is degraded to just a pointer
    int arrayIdx = pin * PCB::numMultiplexors + multiplexorIdx;
    int key = pinToChar[arrayIdx];
    if (key == KEY_EMPTY) {
        return;
    }

    // only apply action on the key if it's not already applied
    if (keyState[arrayIdx] != ignoreActionCondition) {
        if (_debug) {
            String keyStr = keySignalToChar(key);
            keyStr = keyStr != "" ? keyStr : String(char(key));
            Serial.println(keyStr + " " + action + "!");
        } else {
            if (isMacro(key)) {
                return;
            }
            if (key == KEY_LEFT_SPACE || key == KEY_RIGHT_SPACE) {
                key = ' ';
            }
            (Keyboard.*actionFn)(key);
        }
        keyState[arrayIdx] = ignoreActionCondition;
    }
}


void SmallKeyboard::press(String side, byte pin, byte multiplexorIdx) {
    processKey(side, "press", pin, multiplexorIdx);
}

void SmallKeyboard::release(String side, byte pin, byte multiplexorIdx) {
    processKey(side, "release", pin, multiplexorIdx);
}
