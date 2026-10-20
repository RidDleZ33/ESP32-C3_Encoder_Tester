#include <Arduino.h>
#include "config.h"
#include "canbus.h"
#include "encoder.h"
#include "display.h"
#include "buttons.h"
#include "menu.h"
#include "tests.h"

void setup() {
    Serial.begin(115200);
    delay(500);
    Serial.println("\n--- AMARC Encoder Tester ---");

    displayInit();
    displaySplash();

    buttonsInit();
    encoderInit();
    menuInit();

    if (canbusInit()) {
        Serial.println("TWAI 250k OK");
        displayMessage("CAN 250k", "OK");
    } else {
        Serial.println("TWAI init FAILED");
        displayMessage("CAN FAIL", "");
    }
    delay(800);

    // Global NMT start
    canbusSendNmt(0x01, 0x00);
    delay(200);
}

void loop() {
    canbusRecovery();

    // Receive frames
    twai_message_t rx;
    while (canbusReceive(rx)) {
        encoderProcessFrame(rx);
    }

    encoderTimeoutCheck();

    // SYNC when configured
    static uint32_t lastSync = 0;
    if ((enc1.configured || enc2.configured) && (millis() - lastSync > SYNC_INTERVAL_MS)) {
        canbusSendSync();
        lastSync = millis();
    }

    // UI
    static uint32_t lastUI = 0;
    if (millis() - lastUI > UI_UPDATE_MS) {
        Button btn = buttonsGetPress();
        AppState st = menuGetState();

        if (st == STATE_TEST) {
            testRun(menuGetTestIndex(), btn);
        } else {
            menuUpdate(btn);
        }
        lastUI = millis();
    }
}
