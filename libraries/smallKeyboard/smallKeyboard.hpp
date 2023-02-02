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

    // returns true if key is pressed
    bool isKeyPressed(String side, byte pin, byte multiplexorIdx) const;

    // returns the number of key presses
    uint16_t numKeyPress() const { return _numKeyPress; }
    void setKeyPress(uint16_t num) { _numKeyPress = num; }
    void resetKeyPress() { _numKeyPress = 0; }

    // sends the key press signal to computer, maintaining key state
    void press(String side, byte pin, byte multiplexorIdx);

    // sends the key release signal to computer, maintaining key state
    void release(String side, byte pin, byte multiplexorIdx);

    template<typename BaseLED>
    friend class LED;

  private:
    // only print to serial if true
    bool _debug{false};
    bool _leftKeyState[PCB::numMultiplexorReadPins][PCB::numMultiplexors];
    bool _rightKeyState[PCB::numMultiplexorReadPins][PCB::numMultiplexors];
    uint16_t _numKeyPress{0};

    // helper to process a key press or key release
    void processKey(String side, String action, byte pin, byte multiplexorIdx);
    // release all keys that are currently pressed
    void releaseAllPressedKeys();
    // check if this key is macro and cannot be sent to computer directly
    bool isMacro(int key);

    // remap any keys
    int remapKeys(int key) {
        switch (key) {
          case KEY_LEFT_SPACE:
          case KEY_RIGHT_SPACE:
              return ' ';
          // TODO: add your own remapped keys here
          case KEY_CAPS_LOCK:
              return KEY_LEFT_CTRL;
          default:
              return key;
        }
    }
};

#endif // _SMALL_KEYBOARD_HPP

