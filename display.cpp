#include "display.h"
#include "config.h"
#include "encoder.h"
#include <Wire.h>
#include <stdio.h>

U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, U8X8_PIN_NONE);

void displayInit() {
    Wire.begin(PIN_SDA, PIN_SCL);
    u8g2.begin();
    u8g2.setContrast(NORMAL_CONTRAST);
    u8g2.setFont(u8g2_font_6x10_tf);
}

void displaySplash() {
    u8g2.clearBuffer();
    u8g2.drawStr(30, 20, "AMARC");
    u8g2.drawStr(10, 40, "Encoder Tester");
    u8g2.drawStr(50, 55, "v0.2");
    u8g2.sendBuffer();
    delay(SPLASH_MS);
}

void displayMain() {
    u8g2.clearBuffer();
    u8g2.setFont(u8g2_font_6x10_tf);
    u8g2.drawStr(0, 10, "AMARC Encoder Tester");
    u8g2.drawHLine(0, 12, 128);

    char buf[22];
    if (enc1.connected) {
        snprintf(buf, sizeof(buf), "E1 ID:%d | %.1f deg", enc1.nodeId, encoderAngle(enc1));
        u8g2.drawStr(0, 28, buf);
    } else {
        u8g2.drawStr(0, 28, "E1 OFF");
    }

    if (enc2.connected) {
        snprintf(buf, sizeof(buf), "E2 ID:%d | %.1f deg", enc2.nodeId, encoderAngle(enc2));
        u8g2.drawStr(0, 44, buf);
    } else {
        u8g2.drawStr(0, 44, "E2 OFF");
    }

    u8g2.drawStr(0, 63, "CENTER = Menu");
    u8g2.sendBuffer();
}

void displayMenu(const char* title, const char* items[], int count, int selected) {
    u8g2.clearBuffer();
    u8g2.drawStr(0, 10, title);
    u8g2.drawHLine(0, 12, 128);

    int start = 0;
    if (selected >= 4) start = selected - 3;
    if (start + 5 > count) start = count > 5 ? count - 5 : 0;

    for (int i = 0; i < 5 && (start + i) < count; i++) {
        int idx = start + i;
        int y = 24 + i * 10;
        if (idx == selected) {
            u8g2.drawBox(0, y - 8, 128, 10);
            u8g2.setDrawColor(0);
            u8g2.drawStr(2, y, items[idx]);
            u8g2.setDrawColor(1);
        } else {
            u8g2.drawStr(2, y, items[idx]);
        }
    }
    u8g2.sendBuffer();
}

void displayInfo(int scroll) {
    const int MAX_LINES = 20;
    char lines[MAX_LINES][22];
    int n = 0;

    auto add = [&](const char* s) {
        if (n < MAX_LINES) {
            snprintf(lines[n], sizeof(lines[n]), "%s", s);
            n++;
        }
    };

    char buf[22];
    if (enc1.connected) {
        if (!enc1.infoLoaded) encoderLoadInfo(enc1);
        add("--- E1 ---");
        snprintf(buf, sizeof(buf), "Node: %d", enc1.nodeId);
        add(buf);
        snprintf(buf, sizeof(buf), "Scale: %s", (enc1.operatingParams & 0x04) ? "ON" : "OFF");
        add(buf);
        snprintf(buf, sizeof(buf), "Units/r:%lu", enc1.unitsPerRev);
        add(buf);
        snprintf(buf, sizeof(buf), "TotRng:%lu", enc1.totalRange);
        add(buf);
        snprintf(buf, sizeof(buf), "Res:  %lu", enc1.resolution);
        add(buf);
        snprintf(buf, sizeof(buf), "Err:  %02X", enc1.errorReg);
        add(buf);
        add(enc1.deviceName[0] ? enc1.deviceName : "No name");
    } else {
        add("E1: not connected");
    }

    if (enc2.connected) {
        if (!enc2.infoLoaded) encoderLoadInfo(enc2);
        add("--- E2 ---");
        snprintf(buf, sizeof(buf), "Node: %d", enc2.nodeId);
        add(buf);
        snprintf(buf, sizeof(buf), "Scale: %s", (enc2.operatingParams & 0x04) ? "ON" : "OFF");
        add(buf);
        snprintf(buf, sizeof(buf), "Units/r:%lu", enc2.unitsPerRev);
        add(buf);
        snprintf(buf, sizeof(buf), "TotRng:%lu", enc2.totalRange);
        add(buf);
        snprintf(buf, sizeof(buf), "Res:  %lu", enc2.resolution);
        add(buf);
        snprintf(buf, sizeof(buf), "Err:  %02X", enc2.errorReg);
        add(buf);
        add(enc2.deviceName[0] ? enc2.deviceName : "No name");
    } else {
        add("E2: not connected");
    }

    if (scroll < 0) scroll = 0;
    if (n > 5 && scroll > n - 5) scroll = n - 5;

    u8g2.clearBuffer();
    u8g2.setFont(u8g2_font_6x10_tf);
    u8g2.drawStr(0, 10, "Encoder Info");
    u8g2.drawHLine(0, 12, 128);

    for (int i = 0; i < 5 && (scroll + i) < n; i++) {
        u8g2.drawStr(0, 24 + i * 10, lines[scroll + i]);
    }
    u8g2.sendBuffer();
}

void displayMessage(const char* line1, const char* line2) {
    u8g2.clearBuffer();
    if (line1) u8g2.drawStr(0, 30, line1);
    if (line2) u8g2.drawStr(0, 45, line2);
    u8g2.sendBuffer();
}
