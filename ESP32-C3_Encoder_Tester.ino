#include <Arduino.h>
#include "config.h"
#include "canbus.h"
#include "encoder.h"
#include "display.h"
#include "buttons.h"
#include "menu.h"
#include "tests.h"
#include "can_debug.h"

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
        if (canDebugIsLogging()) {
            canDebugLogRx(rx);
        }
        encoderProcessFrame(rx);
        // Mark the encoder currently under sweep test
        if (testIsActive() && testGetIndex() == 0) {
            testSweepOnPosition(0);
        }
    }

    encoderTimeoutCheck();

    // SYNC when configured (pause during CAN debug capture)
    static uint32_t lastSync = 0;
    if (!canDebugIsLogging() && (enc1.configured || enc2.configured) && (millis() - lastSync > SYNC_INTERVAL_MS)) {
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
