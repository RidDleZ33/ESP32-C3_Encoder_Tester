#include "menu.h"
#include "display.h"
#include "encoder.h"
#include "canbus.h"
#include "can_debug.h"
#include <stdio.h>

static AppState state = STATE_MAIN;
static int menuSel = 0;
static int configSel = 0;
static int testSel = 0;
static int infoScroll = 0;
static bool rolesSwapped = false;
static char configStatus[22] = "";
static uint32_t configStatusUntil = 0;

static const char* mainMenu[] = {"Info", "Config", "Sweep Test", "Etch-a-Sketch", "Move+Dir", "Status", "CAN Debug"};
static const int mainMenuCount = 7;

void menuInit() {
    state = STATE_MAIN;
    menuSel = 0;
    rolesSwapped = false;
}

AppState menuGetState() { return state; }
void menuSetState(AppState s) { state = s; }
int menuGetTestIndex() { return testSel; }

static void resetMultiturn(EncoderData& enc) {
    if (!enc.connected || enc.nodeId == 0) {
        snprintf(configStatus, sizeof(configStatus), "E not connected");
        configStatusUntil = millis() + 1200;
        return;
    }
    // CiA 406 object 0x6003: preset current position to 0
    canbusSendSdoWrite(enc.nodeId, 0x6003, 0x00, 0, 4);
    snprintf(configStatus, sizeof(configStatus), "MT reset sent");
    configStatusUntil = millis() + 1200;
}

void menuUpdate(Button btn) {
    switch (state) {
        case STATE_MAIN:
            if (btn == BTN_CENTER) {
                state = STATE_MENU;
                menuSel = 0;
            }
            displayMain();
            break;

        case STATE_MENU:
            if (btn == BTN_UP && menuSel > 0) menuSel--;
            if (btn == BTN_DOWN && menuSel < mainMenuCount - 1) menuSel++;
            if (btn == BTN_CENTER) {
                if (menuSel == 0) { state = STATE_INFO; infoScroll = 0; }
                else if (menuSel == 1) { state = STATE_CONFIG; configSel = 0; }
                else if (menuSel == 6) { state = STATE_CAN_DEBUG; canDebugEnter(); }
                else { state = STATE_TEST; testSel = menuSel - 2; }
            }
            if (btn == BTN_LEFT) state = STATE_MAIN;
            displayMenu("Menu", mainMenu, mainMenuCount, menuSel);
            break;

        case STATE_INFO:
            if (btn == BTN_UP && infoScroll > 0) infoScroll--;
            if (btn == BTN_DOWN) infoScroll++;
            displayInfo(infoScroll);
            if (btn == BTN_CENTER || btn == BTN_LEFT) state = STATE_MENU;
            break;

        case STATE_CONFIG: {
            const int cfgCount = 6;
            if (btn == BTN_UP && configSel > 0) configSel--;
            if (btn == BTN_DOWN && configSel < cfgCount - 1) configSel++;
            if (btn == BTN_CENTER) {
                if (configSel == 0) {
                    EncoderData tmp = enc1;
                    enc1 = enc2;
                    enc2 = tmp;
                    rolesSwapped = !rolesSwapped;
                } else if (configSel == 1) {
                    enc1.invert = !enc1.invert;
                } else if (configSel == 2) {
                    enc2.invert = !enc2.invert;
                } else if (configSel == 3) {
                    resetMultiturn(enc1);
                } else if (configSel == 4) {
                    resetMultiturn(enc2);
                } else if (configSel == 5) {
                    state = STATE_MENU;
                }
            }
            if (btn == BTN_LEFT) state = STATE_MENU;

            char l0[22], l1[22], l2[22], l3[22], l4[22];
            snprintf(l0, sizeof(l0), "Swap Roles: %s", rolesSwapped ? "YES" : "NO");
            snprintf(l1, sizeof(l1), "Invert E1: %s", enc1.invert ? "YES" : "NO");
            snprintf(l2, sizeof(l2), "Invert E2: %s", enc2.invert ? "YES" : "NO");
            snprintf(l3, sizeof(l3), "Reset MT E1");
            snprintf(l4, sizeof(l4), "Reset MT E2");
            const char* cfgItems[] = { l0, l1, l2, l3, l4, "Back" };
            displayMenu("Config", cfgItems, cfgCount, configSel);

            if (configStatus[0] && millis() < configStatusUntil) {
                u8g2.setDrawColor(0);
                u8g2.drawBox(0, 54, 128, 10);
                u8g2.setDrawColor(1);
                u8g2.drawStr(2, 63, configStatus);
                u8g2.sendBuffer();
            }
            break;
        }

        case STATE_TEST:
            break;

        case STATE_CAN_DEBUG:
            canDebugUpdate(btn);
            break;
    }
}
