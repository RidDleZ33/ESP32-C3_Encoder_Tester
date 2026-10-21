#include "display.h"
#include "config.h"
#include "encoder.h"
#include <Wire.h>
#include <stdio.h>

// Full framebuffer — draw off-screen, then send once. No flicker.
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
    u8g2.drawStr(0, 10, "AMARC Tester");
    u8g2.drawHLine(0, 12, 128);

    char buf[20];
    if (enc1.connected) {
        snprintf(buf, sizeof(buf), "E1 ID:%d", enc1.nodeId);
        u8g2.drawStr(0, 26, buf);
        snprintf(buf, sizeof(buf), "  %.1f deg", encoderAngle(enc1));
        u8g2.drawStr(0, 36, buf);
    } else {
        u8g2.drawStr(0, 26, "E1 OFF");
    }

    if (enc2.connected) {
        snprintf(buf, sizeof(buf), "E2 ID:%d", enc2.nodeId);
        u8g2.drawStr(0, 48, buf);
        snprintf(buf, sizeof(buf), "  %.1f deg", encoderAngle(enc2));
        u8g2.drawStr(0, 58, buf);
    } else {
        u8g2.drawStr(0, 48, "E2 OFF");
    }

    u8g2.drawStr(0, 63, "CENTER = Menu");
    u8g2.sendBuffer();
}

void displayMenu(const char* title, const char* items[], int count, int selected) {
    u8g2.clearBuffer();
    u8g2.drawStr(0, 10, title);
    u8g2.drawHLine(0, 12, 128);

    for (int i = 0; i < count && i < 6; i++) {
        int y = 24 + i * 10;
        if (i == selected) {
            u8g2.drawBox(0, y - 8, 128, 10);
            u8g2.setDrawColor(0);
            u8g2.drawStr(2, y, items[i]);
            u8g2.setDrawColor(1);
        } else {
            u8g2.drawStr(2, y, items[i]);
        }
    }
    u8g2.sendBuffer();
}

void displayInfo() {
    u8g2.clearBuffer();
    u8g2.drawStr(0, 10, "Encoder Info");
    u8g2.drawHLine(0, 12, 128);

    char buf[22];
    if (enc1.connected) {
        if (!enc1.infoLoaded) encoderLoadInfo(enc1);
        snprintf(buf, sizeof(buf), "Node: %d", enc1.nodeId);
        u8g2.drawStr(0, 24, buf);
        snprintf(buf, sizeof(buf), "Type: %08lX", enc1.deviceType);
        u8g2.drawStr(0, 34, buf);
        snprintf(buf, sizeof(buf), "Res:  %lu", enc1.resolution);
        u8g2.drawStr(0, 44, buf);
        snprintf(buf, sizeof(buf), "Err:  %02X", enc1.errorReg);
        u8g2.drawStr(0, 54, buf);
        u8g2.drawStr(0, 63, enc1.deviceName[0] ? enc1.deviceName : "No name");
    } else {
        u8g2.drawStr(0, 36, "No encoder");
    }
    u8g2.sendBuffer();
}

void displayMessage(const char* line1, const char* line2) {
    u8g2.clearBuffer();
    if (line1) u8g2.drawStr(0, 30, line1);
    if (line2) u8g2.drawStr(0, 45, line2);
    u8g2.sendBuffer();
}
