#include "can_debug.h"
#include "display.h"
#include "encoder.h"
#include "menu.h"
#include <stdio.h>
#include <string.h>

#define LOG_LINES 24
#define LOG_WIDTH 21

static char logBuf[LOG_LINES][LOG_WIDTH + 1];
static int logCount = 0;
static int logScroll = 0;
static bool logging = false;
static bool stopped = false;

static void addLogLine(const char* line) {
    if (logCount >= LOG_LINES) {
        for (int i = 0; i < LOG_LINES - 1; i++) {
            strcpy(logBuf[i], logBuf[i + 1]);
        }
        logCount = LOG_LINES - 1;
    }
    strncpy(logBuf[logCount], line, LOG_WIDTH);
    logBuf[logCount][LOG_WIDTH] = '\0';
    logCount++;
    if (logCount > 5) logScroll = logCount - 5;
    else logScroll = 0;
}

static const char* noteFor(uint32_t id, const uint8_t* data, uint8_t len, bool tx) {
    if (id == 0x000) {
        if (len >= 1 && data) {
            switch (data[0]) {
                case 0x01: return "NMT start";
                case 0x02: return "NMT stop";
                case 0x80: return "NMT pre-op";
                case 0x81: return "NMT reset";
                case 0x82: return "NMT rst-com";
            }
        }
        return "NMT";
    }
    if (id == 0x080) return "SYNC";
    if ((id & 0x780) == 0x700) return (len == 1 && data && data[0] == 0) ? "boot-up" : "heartbeat";
    if ((id & 0x780) == 0x580) return "SDO rsp";
    if ((id & 0x780) == 0x600) {
        if (len >= 1 && data) {
            if (data[0] == 0x40) return "SDO read";
            if (data[0] == 0x2F || data[0] == 0x2B || data[0] == 0x23) return "SDO write";
        }
        return "SDO req";
    }
    if ((id & 0x780) == 0x180) return "TPDO1 pos";
    if ((id & 0x780) == 0x280) return "TPDO2";
    if ((id & 0x780) == 0x080) return "EMCY";
    return tx ? "TX" : "RX";
}

void canDebugEnter() {
    logging = true;
    stopped = false;
    logCount = 0;
    logScroll = 0;
    memset(logBuf, 0, sizeof(logBuf));
    addLogLine("Reconnect encoder");
    addLogLine("Logging bus...");
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
    if (len == 0) {
        snprintf(buf, sizeof(buf), "TX %03lX", id);
    } else if (len <= 2) {
        snprintf(buf, sizeof(buf), "TX %03lX %02X%02X", id, data[0], len > 1 ? data[1] : 0);
    } else {
        snprintf(buf, sizeof(buf), "TX %03lX %02X%02X%02X", id, data[0], data[1], data[2]);
    }
    addLogLine(buf);
    addLogLine(noteFor(id, data, len, true));
}

void canDebugLogRx(const twai_message_t& frame) {
    if (!logging) return;
    char buf[LOG_WIDTH + 1];
    uint32_t id = frame.identifier;
    uint8_t len = frame.data_length_code;
    if (len == 0) {
        snprintf(buf, sizeof(buf), "RX %03lX", id);
    } else if (len <= 2) {
        snprintf(buf, sizeof(buf), "RX %03lX %02X%02X", id, frame.data[0], len > 1 ? frame.data[1] : 0);
    } else {
        snprintf(buf, sizeof(buf), "RX %03lX %02X%02X%02X", id, frame.data[0], frame.data[1], frame.data[2]);
    }
    addLogLine(buf);
    addLogLine(noteFor(id, frame.data, len, false));

    if (!stopped && (id & 0x780) == 0x180 && len >= 2) {
        logging = false;
        stopped = true;
        addLogLine("--- TPDO stop ---");
    }
}

void canDebugUpdate(Button btn) {
    if (btn == BTN_LEFT || btn == BTN_CENTER) {
        logging = false;
        menuSetState(STATE_MENU);
        return;
    }
    if (btn == BTN_UP && logScroll > 0) logScroll--;
    if (btn == BTN_DOWN && logScroll < logCount - 5 && logCount > 5) logScroll++;

    u8g2.clearBuffer();
    u8g2.setFont(u8g2_font_6x10_tf);
    u8g2.drawStr(0, 10, stopped ? "CAN Debug (done)" : "CAN Debug");
    u8g2.drawHLine(0, 12, 128);

    int start = logScroll;
    for (int i = 0; i < 5 && (start + i) < logCount; i++) {
        u8g2.drawStr(0, 24 + i * 10, logBuf[start + i]);
    }
    if (logCount == 0) {
        u8g2.drawStr(0, 36, "Waiting...");
    }
    if (logCount > 5) {
        u8g2.drawStr(100, 63, logScroll > 0 ? "^v" : " v");
    }
    u8g2.sendBuffer();
}
