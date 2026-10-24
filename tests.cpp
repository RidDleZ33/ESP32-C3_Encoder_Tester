#include "tests.h"
#include "display.h"
#include "encoder.h"
#include "menu.h"
#include "driver/twai.h"
#include <stdio.h>
#include <string.h>

static bool active = false;
static int currentTest = -1;

static float sweepMin = 999, sweepMax = -999;
static int etchX = 64, etchY = 32;
static float movePos = 0;
static float dirAngle = 0;

// Which encoder the sweep is testing (1 or 2). 0 = not chosen yet.
static int sweepWhich = 0;
static bool sweepSelecting = false;
static int sweepPick = 0;

#define SWEEP_MAX_STEPS 16384
static uint8_t sweepSeen[SWEEP_MAX_STEPS / 8];
static uint32_t sweepSeenCount = 0;
static uint32_t sweepSteps = SWEEP_MAX_STEPS;

bool testIsActive() { return active; }
int testGetIndex() { return currentTest; }

void testExit() {
    active = false;
    currentTest = -1;
    sweepSelecting = false;
    sweepWhich = 0;
    menuSetState(STATE_MENU);
}

static EncoderData* sweepEnc() {
    return (sweepWhich == 2) ? &enc2 : &enc1;
}

void testSweepOnPosition(uint32_t rawPosition) {
    if (!active || currentTest != 0 || sweepSelecting || sweepWhich == 0) return;

    EncoderData* enc = sweepEnc();
    uint32_t single = enc->singleTurn;
    uint32_t steps = sweepSteps ? sweepSteps : SWEEP_MAX_STEPS;

    if (single == 0 && rawPosition != 0 && rawPosition < steps) {
        single = rawPosition;
    }
    if (single >= steps || single >= SWEEP_MAX_STEPS) return;

    uint32_t byteIdx = single >> 3;
    uint8_t bit = 1u << (single & 7);
    if (!(sweepSeen[byteIdx] & bit)) {
        sweepSeen[byteIdx] |= bit;
        sweepSeenCount++;
    }
}

static void sweepBegin(int which) {
    sweepWhich = which;
    sweepSelecting = false;
    sweepMin = 999;
    sweepMax = -999;
    memset(sweepSeen, 0, sizeof(sweepSeen));
    sweepSeenCount = 0;
    EncoderData* enc = sweepEnc();
    uint32_t res = enc->resolution;
    sweepSteps = (res > 0 && res <= SWEEP_MAX_STEPS) ? res : SWEEP_MAX_STEPS;
}

void testSweep(Button btn) {
    if (sweepSelecting) {
        if (btn == BTN_UP || btn == BTN_DOWN) sweepPick = 1 - sweepPick;
        if (btn == BTN_LEFT) { testExit(); return; }
        if (btn == BTN_CENTER) {
            sweepBegin(sweepPick == 0 ? 1 : 2);
            btn = BTN_NONE;
        } else {
            u8g2.clearBuffer();
            u8g2.setFont(u8g2_font_6x10_tf);
            u8g2.drawStr(0, 10, "Sweep which?");
            u8g2.drawHLine(0, 12, 128);
            if (sweepPick == 0) {
                u8g2.drawBox(0, 16, 128, 12);
                u8g2.setDrawColor(0);
                u8g2.drawStr(2, 26, "E1");
                u8g2.setDrawColor(1);
                u8g2.drawStr(2, 42, "E2");
            } else {
                u8g2.drawStr(2, 26, "E1");
                u8g2.drawBox(0, 32, 128, 12);
                u8g2.setDrawColor(0);
                u8g2.drawStr(2, 42, "E2");
                u8g2.setDrawColor(1);
            }
            u8g2.drawStr(0, 63, "CENTER=select");
            u8g2.sendBuffer();
            return;
        }
    }

    if (btn == BTN_CENTER || btn == BTN_LEFT) { testExit(); return; }

    EncoderData* enc = sweepEnc();
    float a = 0;
    uint32_t raw = 0;
    uint32_t single = 0;
    if (enc->connected) {
        a = encoderAngle(*enc);
        raw = enc->rawPosition;
        single = enc->singleTurn;
        if (a < sweepMin) sweepMin = a;
        if (a > sweepMax) sweepMax = a;
    }

    char title[22];
    bool passed = (sweepSteps > 0 && sweepSeenCount >= sweepSteps);
    if (passed) {
        snprintf(title, sizeof(title), "E%d TEST PASS", sweepWhich);
    } else {
        snprintf(title, sizeof(title), "Sweep E%d (slow!)", sweepWhich);
    }

    u8g2.clearBuffer();
    u8g2.setFont(u8g2_font_6x10_tf);
    u8g2.drawStr(0, 10, title);
    u8g2.drawHLine(0, 12, 128);

    char buf[22];
    snprintf(buf, sizeof(buf), "Ang:%5.1f ST:%lu", a, single);
    u8g2.drawStr(0, 24, buf);
    snprintf(buf, sizeof(buf), "Abs:%lu", raw % 1000000UL);
    u8g2.drawStr(0, 36, buf);
    snprintf(buf, sizeof(buf), "Seen:%lu/%lu", sweepSeenCount, sweepSteps);
    u8g2.drawStr(0, 48, buf);
    snprintf(buf, sizeof(buf), "Rng:%.0f-%.0f", sweepMin > 900 ? 0 : sweepMin, sweepMax < -900 ? 0 : sweepMax);
    u8g2.drawStr(0, 58, buf);
    u8g2.drawStr(0, 63, "CENTER=exit");
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
        if (index == 0) {
            if (enc1.connected && enc2.connected) {
                sweepSelecting = true;
                sweepPick = 0;
                sweepWhich = 0;
            } else if (enc2.connected) {
                sweepBegin(2);
            } else {
                sweepBegin(1);
            }
        }
    }
    switch (index) {
        case 0: testSweep(btn); break;
        case 1: testEtch(btn); break;
        case 2: testMoveDir(btn); break;
        case 3: testStatus(btn); break;
        default: testExit(); break;
    }
}
