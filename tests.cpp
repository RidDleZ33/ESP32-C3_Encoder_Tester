#include "tests.h"
#include "display.h"
#include "encoder.h"
#include "menu.h"
#include "driver/twai.h"
#include <stdio.h>

static bool active = false;
static int currentTest = -1;

// Sweep
static float sweepMin = 999, sweepMax = -999;

// Etch
static int etchX = 64, etchY = 32;

// Move+Dir
static float movePos = 0;
static float dirAngle = 0;

bool testIsActive() { return active; }

void testExit() {
    active = false;
    currentTest = -1;
    menuSetState(STATE_MENU);
}

void testSweep(Button btn) {
    if (btn == BTN_CENTER || btn == BTN_LEFT) { testExit(); return; }
    float a = enc1.connected ? encoderAngle(enc1) : 0;
    if (a < sweepMin) sweepMin = a;
    if (a > sweepMax) sweepMax = a;

    u8x8.clearDisplay();
    // Title in yellow
    u8x8.drawString(0, 0, "Sweep Test");
    u8x8.drawString(0, 1, "-------------");

    char buf[18];
    snprintf(buf, sizeof(buf), "Now: %6.1f", a);
    u8x8.drawString(0, 2, buf);
    snprintf(buf, sizeof(buf), "Min: %6.1f", sweepMin);
    u8x8.drawString(0, 4, buf);
    snprintf(buf, sizeof(buf), "Max: %6.1f", sweepMax);
    u8x8.drawString(0, 6, buf);
    u8x8.drawString(0, 7, "CENTER = exit");
}

void testEtch(Button btn) {
    if (btn == BTN_CENTER || btn == BTN_LEFT) { testExit(); return; }
    if (enc1.connected) etchX = (int)(encoderAngle(enc1) / 360.0f * 120);
    if (enc2.connected) etchY = (int)(encoderAngle(enc2) / 360.0f * 60);

    u8x8.clearDisplay();
    // Title in yellow
    u8x8.drawString(0, 0, "Etch-a-Sketch");
    u8x8.drawString(0, 1, "-------------");

    char buf[18];
    snprintf(buf, sizeof(buf), "X: %3d", etchX);
    u8x8.drawString(0, 3, buf);
    snprintf(buf, sizeof(buf), "Y: %3d", etchY);
    u8x8.drawString(0, 5, buf);
    u8x8.drawString(0, 7, "CENTER = exit");
}

void testMoveDir(Button btn) {
    if (btn == BTN_CENTER || btn == BTN_LEFT) { testExit(); return; }
    if (enc1.connected) movePos = encoderAngle(enc1);
    if (enc2.connected) dirAngle = encoderAngle(enc2);

    u8x8.clearDisplay();
    // Title in yellow
    u8x8.drawString(0, 0, "Move + Dir");
    u8x8.drawString(0, 1, "-------------");

    char buf[18];
    snprintf(buf, sizeof(buf), "Move: %6.1f", movePos);
    u8x8.drawString(0, 3, buf);
    snprintf(buf, sizeof(buf), "Dir:  %6.1f", dirAngle);
    u8x8.drawString(0, 5, buf);
    u8x8.drawString(0, 7, "CENTER = exit");
}

void testStatus(Button btn) {
    if (btn == BTN_CENTER || btn == BTN_LEFT) { testExit(); return; }

    u8x8.clearDisplay();
    // Title in yellow
    u8x8.drawString(0, 0, "Status");
    u8x8.drawString(0, 1, "-------------");

    char buf[18];
    snprintf(buf, sizeof(buf), "E1: %s", enc1.connected ? "ON" : "OFF");
    u8x8.drawString(0, 2, buf);
    snprintf(buf, sizeof(buf), "E2: %s", enc2.connected ? "ON" : "OFF");
    u8x8.drawString(0, 3, buf);

    twai_status_info_t st;
    if (twai_get_status_info(&st) == ESP_OK) {
        snprintf(buf, sizeof(buf), "Bus: %d", st.state);
        u8x8.drawString(0, 5, buf);
        snprintf(buf, sizeof(buf), "Err: %u", st.bus_error_count);
        u8x8.drawString(0, 6, buf);
    }
    u8x8.drawString(0, 7, "CENTER = exit");
}

void testRun(int index, Button btn) {
    if (!active) {
        active = true;
        currentTest = index;
        sweepMin = 999; sweepMax = -999;
    }
    switch (index) {
        case 0: testSweep(btn); break;
        case 1: testEtch(btn); break;
        case 2: testMoveDir(btn); break;
        case 3: testStatus(btn); break;
        default: testExit(); break;
    }
}
