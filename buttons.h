#pragma once
#include <Arduino.h>

enum Button {
    BTN_NONE = 0,
    BTN_UP,
    BTN_DOWN,
    BTN_LEFT,
    BTN_RIGHT,
    BTN_CENTER
};

void buttonsInit();
Button buttonsRead();          // raw
Button buttonsGetPress();      // debounced edge (press only)
