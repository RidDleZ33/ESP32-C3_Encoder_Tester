#include "tests.h"
#include "display.h"
#include "encoder.h"
#include "menu.h"
#include "driver/twai.h"
#include <stdio.h>

static bool active = false;
static int currentTest = -1;

static float sweepMin = 999, sweepMax = -999;
static int etchX = 64, etchY = 32;
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

    u8g2.clearBuffer();
    u8g2.drawStr(0, 10, "Sweep Test");
    u8g2.drawHLine(0, 12, 128);

    char buf[20];
    snprintf(buf, sizeof(buf), "Now: %6.1f", a);
    u8g2.drawStr(0, 26, buf);
    snprintf(buf, sizeof(buf), "Min: %6.1f", sweepMin);
    u8g2.drawStr(0, 40, buf);
    snprintf(buf, sizeof(buf), "Max: %6.1f", sweepMax);
    u8g2.drawStr(0, 54, buf);
    u8g2.drawStr(0, 63, "CENTER = exit");
    u8g2.sendBuffer();
}

void testEtch(Button btn) {
    if (btn == BTN_CENTER || btn == BTN_LEFT) { testExit(); return; }
    if (enc1.connected) etchX = (int)(encoderAngle(enc1) / 360.0f * 120);
    if (enc2.connected) etchY = (int)(encoderAngle(enc2) / 360.0f * 60);

    u8g2.clearBuffer();
    u8g2.drawStr(0, 10, "Etch-a-Sketch");
    u8g2.drawHLine(0, 12, 128);

    char buf[20];
    snprintf(buf, sizeof(buf), "X: %3d", etchX);
    u8g2.drawStr(0, 32, buf);
    snprintf(buf, sizeof(buf), "Y: %3d", etchY);
    u8g2.drawStr(0, 48, buf);
    u8g2.drawStr(0, 63, "CENTER = exit");
    u8g2.sendBuffer();
}

void testMoveDir(Button btn) {
    if (btn == BTN_CENTER || btn == BTN_LEFT) { testExit(); return; }
    if (enc1.connected) movePos = encoderAngle(enc1);
    if (enc2.connected) dirAngle = encoderAngle(enc2);

    u8g2.clearBuffer();
    u8g2.drawStr(0, 10, "Move + Dir");
    u8g2.drawHLine(0, 12, 128);

    char buf[20];
    snprintf(buf, sizeof(buf), "Move: %6.1f", movePos);
    u8g2.drawStr(0, 32, buf);
    snprintf(buf, sizeof(buf), "Dir:  %6.1f", dirAngle);
    u8g2.drawStr(0, 48, buf);
    u8g2.drawStr(0, 63, "CENTER = exit");
    u8g2.sendBuffer();
}

void testStatus(Button btn) {
    if (btn == BTN_CENTER || btn == BTN_LEFT) { testExit(); return; }

    u8g2.clearBuffer();
    u8g2.drawStr(0, 10, "Status");
    u8g2.drawHLine(0, 12, 128);

    char buf[20];
    snprintf(buf, sizeof(buf), "E1: %s", enc1.connected ? "ON" : "OFF");
    u8g2.drawStr(0, 26, buf);
    snprintf(buf, sizeof(buf), "E2: %s", enc2.connected ? "ON" : "OFF");
    u8g2.drawStr(0, 36, buf);

    twai_status_info_t st;
    if (twai_get_status_info(&st) == ESP_OK) {
        snprintf(buf, sizeof(buf), "Bus: %d", st.state);
        u8g2.drawStr(0, 50, buf);
        snprintf(buf, sizeof(buf), "Err: %u", st.bus_error_count);
        u8g2.drawStr(0, 60, buf);
    }
    u8g2.sendBuffer();
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
