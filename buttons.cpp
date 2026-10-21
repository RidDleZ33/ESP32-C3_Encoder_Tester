#include "buttons.h"
#include "config.h"

void buttonsInit() {
    pinMode(BTN_UP_PIN, INPUT_PULLUP);
    pinMode(BTN_DOWN_PIN, INPUT_PULLUP);
    pinMode(BTN_LEFT_PIN, INPUT_PULLUP);
    pinMode(BTN_RIGHT_PIN, INPUT_PULLUP);
    pinMode(BTN_CENTER_PIN, INPUT_PULLUP);
}

// Module orientation: previous CCW rotation was backwards.
// This mapping makes physical up/down/left/right match the labels.
Button buttonsRead() {
    if (digitalRead(BTN_UP_PIN) == LOW) return BTN_RIGHT;
    if (digitalRead(BTN_DOWN_PIN) == LOW) return BTN_LEFT;
    if (digitalRead(BTN_LEFT_PIN) == LOW) return BTN_UP;
    if (digitalRead(BTN_RIGHT_PIN) == LOW) return BTN_DOWN;
    if (digitalRead(BTN_CENTER_PIN) == LOW) return BTN_CENTER;
    return BTN_NONE;
}

Button buttonsGetPress() {
    static Button last = BTN_NONE;
    static uint32_t lastMs = 0;
    Button cur = buttonsRead();
    uint32_t now = millis();
    if (cur != last && (now - lastMs) > BUTTON_DEBOUNCE_MS) {
        last = cur;
        lastMs = now;
        if (cur != BTN_NONE) return cur;
    }
    return BTN_NONE;
}
