#include "can_debug.h"
#include "display.h"
#include "encoder.h"
#include "menu.h"
#include <stdio.h>
#include <string.h>

// Ensure u8x8 is visible (extern from display.h / defined in display.cpp)
extern U8X8_SSD1306_128X64_NONAME_HW_I2C u8x8;

#define LOG_LINES 16
#define LOG_WIDTH 16

static char logBuf[LOG_LINES][LOG_WIDTH + 1];
static int logCount = 0;
static int logScroll = 0;
static bool logging = false;
static bool stopped = false;

static void addLogLine(const char* line) {
    if (!logging) return;
    // Shift if full
    if (logCount >= LOG_LINES) {
        for (int i = 0; i < LOG_LINES - 1; i++) {
            strcpy(logBuf[i], logBuf[i + 1]);
        }
        logCount = LOG_LINES - 1;
    }
    strncpy(logBuf[logCount], line, LOG_WIDTH);
    logBuf[logCount][LOG_WIDTH] = '\0';
    logCount++;
    // Auto-scroll to bottom
    if (logCount > 6) logScroll = logCount - 6;
    else logScroll = 0;
}

void canDebugEnter() {
    logging = true;
    stopped = false;
    logCount = 0;
    logScroll = 0;
    memset(logBuf, 0, sizeof(logBuf));
    addLogLine("Reconnect encoder");
    addLogLine("Logging bus...");
    // Force re-detection on next frames
    enc1.connected = false;
    enc1.configured = false;
    enc1.nodeId = 0;
    enc2.connected = false;
    enc2.configured = false;
    enc2.nodeId = 0;
}

bool canDebugIsLogging() { return logging; }

void canDebugLogTx(uint32_t id, const uint8_t* data, uint8_t len) {
    if (!logging) return;
    char buf[LOG_WIDTH + 1];
    // Short format: TX ID DLC: first few bytes
    if (len == 0) {
        snprintf(buf, sizeof(buf), "TX %03X 0", id);
    } else if (len <= 2) {
        snprintf(buf, sizeof(buf), "TX %03X %02X%02X", id, data[0], len > 1 ? data[1] : 0);
    } else {
        snprintf(buf, sizeof(buf), "TX %03X %02X%02X%02X", id, data[0], data[1], data[2]);
    }
    addLogLine(buf);
}

void canDebugLogRx(const twai_message_t& frame) {
    if (!logging) return;
    char buf[LOG_WIDTH + 1];
    uint32_t id = frame.identifier;
    uint8_t len = frame.data_length_code;
    if (len == 0) {
        snprintf(buf, sizeof(buf), "RX %03X 0", id);
    } else if (len <= 2) {
        snprintf(buf, sizeof(buf), "RX %03X %02X%02X", id, frame.data[0], len > 1 ? frame.data[1] : 0);
    } else {
        snprintf(buf, sizeof(buf), "RX %03X %02X%02X%02X", id, frame.data[0], frame.data[1], frame.data[2]);
    }
    addLogLine(buf);

    // Stop on first TPDO1 (position data) after we have some activity
    if (!stopped && (id & 0x780) == 0x180 && len >= 2) {
        addLogLine("--- TPDO stop ---");
        logging = false;
        stopped = true;
    }
}

void canDebugUpdate(Button btn) {
    if (btn == BTN_LEFT || btn == BTN_CENTER) {
        logging = false;
        menuSetState(STATE_MENU);
        return;
    }
    if (btn == BTN_UP && logScroll > 0) logScroll--;
    if (btn == BTN_DOWN && logScroll < logCount - 6 && logCount > 6) logScroll++;

    // Render
    u8x8.clearDisplay();
    u8x8.drawString(0, 0, stopped ? "CAN Debug (done)" : "CAN Debug");
    int start = logScroll;
    for (int i = 0; i < 6 && (start + i) < logCount; i++) {
        u8x8.drawString(0, i + 1, logBuf[start + i]);
    }
    if (logCount == 0) {
        u8x8.drawString(0, 2, "Waiting...");
    }
    // Scroll hint
    if (logCount > 6) {
        u8x8.drawString(0, 7, logScroll > 0 ? "^v" : " v");
    }
}
