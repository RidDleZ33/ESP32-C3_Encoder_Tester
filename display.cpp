#include "display.h"
#include "config.h"
#include "encoder.h"
#include <Wire.h>
#include <stdio.h>

U8X8_SSD1306_128X64_NONAME_HW_I2C u8x8(U8X8_PIN_NONE);

void displayInit() {
    Wire.begin(PIN_SDA, PIN_SCL);
    u8x8.begin();
    u8x8.setFont(u8x8_font_chroma48medium8_r);
    u8x8.setContrast(NORMAL_CONTRAST);
}

void displaySplash() {
    u8x8.clearDisplay();
    u8x8.drawString(2, 2, "AMARC");
    u8x8.drawString(1, 4, "Encoder Tester");
    u8x8.drawString(3, 6, "v0.2");
    delay(SPLASH_MS);
    u8x8.clearDisplay();
}

void displayMain() {
    u8x8.clearDisplay();
    u8x8.drawString(0, 0, "AMARC Tester");

    char buf[18];
    if (enc1.connected) {
        snprintf(buf, sizeof(buf), "E1 ID:%3d", enc1.nodeId);
        u8x8.drawString(0, 2, buf);
        snprintf(buf, sizeof(buf), "   %6.1f deg", encoderAngle(enc1));
        u8x8.drawString(0, 3, buf);
    } else {
        u8x8.drawString(0, 2, "E1 OFF");
    }

    if (enc2.connected) {
        snprintf(buf, sizeof(buf), "E2 ID:%3d", enc2.nodeId);
        u8x8.drawString(0, 5, buf);
        snprintf(buf, sizeof(buf), "   %6.1f deg", encoderAngle(enc2));
        u8x8.drawString(0, 6, buf);
    } else {
        u8x8.drawString(0, 5, "E2 OFF");
    }

    u8x8.drawString(0, 7, "CENTER = Menu");
}

void displayMenu(const char* title, const char* items[], int count, int selected) {
    u8x8.clearDisplay();
    u8x8.drawString(0, 0, title);
    for (int i = 0; i < count && i < 7; i++) {
        if (i == selected) {
            u8x8.setInverseFont(1);
        }
        char line[17];
        snprintf(line, sizeof(line), "%-16s", items[i]);
        u8x8.drawString(0, i + 1, line);
        u8x8.setInverseFont(0);
    }
}

void displayInfo() {
    u8x8.clearDisplay();
    u8x8.drawString(0, 0, "Encoder Info");
    char buf[18];
    if (enc1.connected) {
        if (!enc1.infoLoaded) encoderLoadInfo(enc1);
        snprintf(buf, sizeof(buf), "Node: %d", enc1.nodeId);
        u8x8.drawString(0, 2, buf);
        snprintf(buf, sizeof(buf), "Type: %08lX", enc1.deviceType);
        u8x8.drawString(0, 3, buf);
        snprintf(buf, sizeof(buf), "Res:  %lu", enc1.resolution);
        u8x8.drawString(0, 4, buf);
        snprintf(buf, sizeof(buf), "Err:  %02X", enc1.errorReg);
        u8x8.drawString(0, 5, buf);
        u8x8.drawString(0, 6, enc1.deviceName[0] ? enc1.deviceName : "No name");
        u8x8.drawString(0, 7, "CENTER=back");
    } else {
        u8x8.drawString(0, 3, "No encoder");
        u8x8.drawString(0, 7, "CENTER=back");
    }
}

void displayMessage(const char* line1, const char* line2) {
    u8x8.clearDisplay();
    if (line1) u8x8.drawString(0, 3, line1);
    if (line2) u8x8.drawString(0, 5, line2);
}
