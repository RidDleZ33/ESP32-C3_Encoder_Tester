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

// Sweep coverage (single-turn absolute positions)
#define SWEEP_MAX_STEPS 4096
static uint8_t sweepSeen[SWEEP_MAX_STEPS / 8];
static uint32_t sweepSeenCount = 0;
static uint32_t sweepSteps = SWEEP_MAX_STEPS;

bool testIsActive() { return active; }
int testGetIndex() { return currentTest; }

void testExit() {
    active = false;
    currentTest = -1;
    menuSetState(STATE_MENU);
}

void testSweepOnPosition(uint32_t rawPosition) {
    if (!active || currentTest != 0) return;

    // Prefer the already-extracted single-turn value when available
    uint32_t single = enc1.singleTurn;
    uint32_t steps = sweepSteps ? sweepSteps : SWEEP_MAX_STEPS;

    // Fallback if singleTurn not yet populated
    if (single == 0 && rawPosition != 0) {
        if ((steps & (steps - 1)) == 0) {
            single = rawPosition & (steps - 1);
        } else {
            single = rawPosition % steps;
        }
    }
    if (single >= SWEEP_MAX_STEPS) return;

    uint32_t byteIdx = single >> 3;
    uint8_t bit = 1u << (single & 7);
    if (!(sweepSeen[byteIdx] & bit)) {
        sweepSeen[byteIdx] |= bit;
        sweepSeenCount++;
    }
}

void testSweep(Button btn) {
    if (btn == BTN_CENTER || btn == BTN_LEFT) { testExit(); return; }

    float a = 0;
    uint32_t raw = 0;
    uint32_t single = 0;
    if (enc1.connected) {
        a = encoderAngle(enc1);
        raw = enc1.rawPosition;
        single = enc1.singleTurn;
        if (a < sweepMin) sweepMin = a;
        if (a > sweepMax) sweepMax = a;
    }

    uint32_t steps = sweepSteps ? sweepSteps : SWEEP_MAX_STEPS;
    float pct = steps ? (100.0f * sweepSeenCount / steps) : 0;

    u8g2.clearBuffer();
    u8g2.setFont(u8g2_font_6x10_tf);
    u8g2.drawStr(0, 10, "Sweep (slow!)");
    u8g2.drawHLine(0, 12, 128);

    char buf[22];
    snprintf(buf, sizeof(buf), "Ang: %5.1f", a);
    u8g2.drawStr(0, 24, buf);
    snprintf(buf, sizeof(buf), "ST: %lu", single);
    u8g2.drawStr(0, 36, buf);
    snprintf(buf, sizeof(buf), "Abs:%lu", raw % 1000000UL);
    u8g2.drawStr(0, 46, buf);
    snprintf(buf, sizeof(buf), "Seen:%lu/%lu %.0f%%", sweepSeenCount, steps, pct);
    u8g2.drawStr(0, 56, buf);
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
            memset(sweepSeen, 0, sizeof(sweepSeen));
            sweepSeenCount = 0;
            // Use reported resolution if it looks like a reasonable single-turn value.
            // Force 4096 when the encoder reports 8192 (physical 180° was mapping to 360°).
            uint32_t res = enc1.resolution;
            if (res == 8192 || res == 0) {
                sweepSteps = 4096;
            } else if (res > 0 && res <= SWEEP_MAX_STEPS) {
                sweepSteps = res;
            } else {
                sweepSteps = SWEEP_MAX_STEPS;
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
