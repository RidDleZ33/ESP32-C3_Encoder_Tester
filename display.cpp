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
    u8x8.drawString(0, 2, "  AMARC");
    u8x8.drawString(0, 4, "Encoder Tester");
    delay(SPLASH_MS);
    u8x8.clearDisplay();
}

void displayMain() {
    u8x8.clearDisplay();
    u8x8.drawString(0, 0, "AMARC Tester");

    char buf[20];
    if (enc1.connected) {
        snprintf(buf, sizeof(buf), "E1 ID:%3d %5.1f", enc1.nodeId, encoderAngle(enc1));
    } else {
        snprintf(buf, sizeof(buf), "E1 OFF");
    }
    u8x8.drawString(0, 2, buf);

    if (enc2.connected) {
        snprintf(buf, sizeof(buf), "E2 ID:%3d %5.1f", enc2.nodeId, encoderAngle(enc2));
    } else {
        snprintf(buf, sizeof(buf), "E2 OFF");
    }
    u8x8.drawString(0, 4, buf);

    u8x8.drawString(0, 6, "CENTER=Menu");
}

void displayMenu(const char* title, const char* items[], int count, int selected) {
    u8x8.clearDisplay();
    u8x8.drawString(0, 0, title);
    for (int i = 0; i < count && i < 6; i++) {
        if (i == selected) {
            u8x8.setInverseFont(1);
        }
        u8x8.drawString(0, i + 1, items[i]);
        u8x8.setInverseFont(0);
    }
}

void displayInfo() {
    u8x8.clearDisplay();
    u8x8.drawString(0, 0, "Encoder Info");
    char buf[22];
    if (enc1.connected) {
        if (!enc1.infoLoaded) encoderLoadInfo(enc1);
        snprintf(buf, sizeof(buf), "ID:%d Type:%08lX", enc1.nodeId, enc1.deviceType);
        u8x8.drawString(0, 2, buf);
        snprintf(buf, sizeof(buf), "Res:%lu Err:%02X", enc1.resolution, enc1.errorReg);
        u8x8.drawString(0, 4, buf);
        u8x8.drawString(0, 6, enc1.deviceName[0] ? enc1.deviceName : "No name");
    } else {
        u8x8.drawString(0, 2, "No encoder");
    }
}

void displayMessage(const char* line1, const char* line2) {
    u8x8.clearDisplay();
    if (line1) u8x8.drawString(0, 2, line1);
    if (line2) u8x8.drawString(0, 4, line2);
}
