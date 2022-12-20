#ifndef _SMALL_KEYBOARD_HPP
#define _SMALL_KEYBOARD_HPP

#include <Keyboard.h>

#include "config.h"
#include "utils.h"

class SmallKeyboard {
  public:
    SmallKeyboard() {}
    void begin() { Keyboard.begin(); }
    void setDebug(bool state) { _debug = state; }

    // sends the key press signal to computer, maintaining key state
    void press(String side, byte pin, byte multiplexorIdx);

    // sends the key release signal to computer, maintaining key state
    void release(String side, byte pin, byte multiplexorIdx);

  private:
    // only print to serial if true
    bool _debug{true};
    bool _leftKeyState[PCB::numMultiplexorReadPins][PCB::numMultiplexors];
    bool _rightKeyState[PCB::numMultiplexorReadPins][PCB::numMultiplexors];

    // helper to process a key press or key release
    void processKey(String side, String action, byte pin, byte multiplexorIdx);
    // check if this key is macro and cannot be sent to computer directly
    bool isMacro(int key);
};

#endif // _SMALL_KEYBOARD_HPP

