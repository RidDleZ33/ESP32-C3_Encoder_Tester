#include "menu.h"
#include "display.h"
#include "encoder.h"
#include "canbus.h"
#include "can_debug.h"
#include <stdio.h>
#include <string.h>

static AppState state = STATE_MAIN;
static int menuSel = 0;
static int configSel = 0;
static int testSel = 0;
static int infoScroll = 0;
static bool rolesSwapped = false;
static char configStatus[22] = "";
static uint32_t configStatusUntil = 0;

static int encParamsWhich = 0;
static int encParamsSel = 0;
static bool encParamsPicking = false;
static int encParamsPick = 0;
static bool encParamsEditing = false;
static int encParamsEditField = 0;
static uint32_t encParamsEditVal = 0;
static int encParamsDigit = 0;
static bool encParamsConfirm = false;
static int encParamsConfirmSel = 0;
static uint32_t pendingOp = 0;
static uint32_t pendingUnits = 0;
static uint32_t pendingTotal = 0;

static const char* mainMenu[] = {"Info", "Config", "Enc Params", "Sweep Test", "Etch-a-Sketch", "Move+Dir", "Status", "CAN Debug"};
static const int mainMenuCount = 8;

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
    canbusSendSdoWrite(enc.nodeId, 0x6003, 0x00, 0, 4);
    snprintf(configStatus, sizeof(configStatus), "MT reset sent");
    configStatusUntil = millis() + 1200;
}

static EncoderData* paramsEnc() {
    return (encParamsWhich == 2) ? &enc2 : &enc1;
}

static void paramsBegin(int which) {
    encParamsWhich = which;
    encParamsPicking = false;
    encParamsSel = 0;
    encParamsEditing = false;
    encParamsConfirm = false;
    EncoderData* e = paramsEnc();
    if (e->connected && !e->infoLoaded) encoderLoadInfo(*e);
    pendingOp = e->operatingParams;
    pendingUnits = e->unitsPerRev;
    pendingTotal = e->totalRange;
}

static void paramsWrite() {
    EncoderData* e = paramsEnc();
    if (!e->connected || e->nodeId == 0) return;
    canbusSendSdoWrite(e->nodeId, 0x6000, 0x00, pendingOp, 2);
    delay(20);
    canbusSendSdoWrite(e->nodeId, 0x6001, 0x00, pendingUnits, 4);
    delay(20);
    canbusSendSdoWrite(e->nodeId, 0x6002, 0x00, pendingTotal, 4);
    delay(20);
    e->infoLoaded = false;
    encoderLoadInfo(*e);
}

static void drawDigitEditor() {
    char num[12];
    snprintf(num, sizeof(num), "%08lu", encParamsEditVal);
    u8g2.clearBuffer();
    u8g2.setFont(u8g2_font_6x10_tf);
    u8g2.drawStr(0, 10, encParamsEditField == 1 ? "Units/rev" : "Total range");
    u8g2.drawHLine(0, 12, 128);

    int startX = 16;
    for (int i = 0; i < 8; i++) {
        int x = startX + i * 12;
        if (i == encParamsDigit) {
            u8g2.drawBox(x - 1, 20, 11, 14);
            u8g2.setDrawColor(0);
            u8g2.drawGlyph(x, 32, num[i]);
            u8g2.setDrawColor(1);
        } else {
            u8g2.drawGlyph(x, 32, num[i]);
        }
    }
    u8g2.drawStr(0, 52, "L/R move  U/D +/-");
    u8g2.drawStr(0, 63, "CENTER=set");
    u8g2.sendBuffer();
}

static void drawConfirm() {
    u8g2.clearBuffer();
    u8g2.setFont(u8g2_font_6x10_tf);
    u8g2.drawStr(0, 10, "Write to encoder?");
    u8g2.drawHLine(0, 12, 128);
    u8g2.drawStr(0, 26, "Previous values");
    u8g2.drawStr(0, 36, "will be lost.");
    if (encParamsConfirmSel == 0) {
        u8g2.drawBox(0, 40, 40, 12);
        u8g2.setDrawColor(0);
        u8g2.drawStr(4, 50, "NO");
        u8g2.setDrawColor(1);
        u8g2.drawStr(50, 50, "YES");
    } else {
        u8g2.drawStr(4, 50, "NO");
        u8g2.drawBox(46, 40, 44, 12);
        u8g2.setDrawColor(0);
        u8g2.drawStr(50, 50, "YES");
        u8g2.setDrawColor(1);
    }
    u8g2.drawStr(0, 63, "CENTER=choose");
    u8g2.sendBuffer();
}

static void encParamsUpdate(Button btn) {
    if (encParamsPicking) {
        if (btn == BTN_UP || btn == BTN_DOWN) encParamsPick = 1 - encParamsPick;
        if (btn == BTN_LEFT) { state = STATE_MENU; return; }
        if (btn == BTN_CENTER) {
            paramsBegin(encParamsPick == 0 ? 1 : 2);
        } else {
            u8g2.clearBuffer();
            u8g2.setFont(u8g2_font_6x10_tf);
            u8g2.drawStr(0, 10, "Params which?");
            u8g2.drawHLine(0, 12, 128);
            if (encParamsPick == 0) {
                u8g2.drawBox(0, 16, 128, 12);
                u8g2.setDrawColor(0);
                u8g2.drawStr(2, 26, "E1");
                u8g2.setDrawColor(1);
                u8g2.drawStr(2, 42, "E2");
            } else {
                u8g2.drawStr(2, 26, "E1");
                u8g2.drawBox(0, 32, 128, 12);
                u8g2.setDrawColor(0);
                u8g2.drawStr(2, 42, "E2");
                u8g2.setDrawColor(1);
            }
            u8g2.drawStr(0, 63, "CENTER=select");
            u8g2.sendBuffer();
            return;
        }
    }

    if (encParamsConfirm) {
        if (btn == BTN_LEFT || btn == BTN_RIGHT || btn == BTN_UP || btn == BTN_DOWN) {
            encParamsConfirmSel = 1 - encParamsConfirmSel;
        }
        if (btn == BTN_CENTER) {
            if (encParamsConfirmSel == 1) {
                paramsWrite();
            }
            encParamsConfirm = false;
        }
        drawConfirm();
        return;
    }

    if (encParamsEditing) {
        if (btn == BTN_LEFT && encParamsDigit > 0) encParamsDigit--;
        if (btn == BTN_RIGHT && encParamsDigit < 7) encParamsDigit++;
        if (btn == BTN_UP || btn == BTN_DOWN) {
            uint32_t place = 1;
            for (int i = 0; i < (7 - encParamsDigit); i++) place *= 10;
            uint32_t d = (encParamsEditVal / place) % 10;
            if (btn == BTN_UP) d = (d + 1) % 10;
            else d = (d == 0) ? 9 : d - 1;
            encParamsEditVal = (encParamsEditVal - ((encParamsEditVal / place) % 10) * place) + d * place;
        }
        if (btn == BTN_CENTER) {
            if (encParamsEditField == 1) pendingUnits = encParamsEditVal;
            else pendingTotal = encParamsEditVal;
            encParamsEditing = false;
        }
        drawDigitEditor();
        return;
    }

    const int nItems = 5;
    if (btn == BTN_UP && encParamsSel > 0) encParamsSel--;
    if (btn == BTN_DOWN && encParamsSel < nItems - 1) encParamsSel++;
    if (btn == BTN_LEFT) { state = STATE_MENU; return; }
    if (btn == BTN_CENTER) {
        if (encParamsSel == 0) {
            pendingOp ^= 0x04;
        } else if (encParamsSel == 1) {
            encParamsEditing = true;
            encParamsEditField = 1;
            encParamsEditVal = pendingUnits;
            encParamsDigit = 0;
        } else if (encParamsSel == 2) {
            encParamsEditing = true;
            encParamsEditField = 2;
            encParamsEditVal = pendingTotal;
            encParamsDigit = 0;
        } else if (encParamsSel == 3) {
            encParamsConfirm = true;
            encParamsConfirmSel = 0;
        } else if (encParamsSel == 4) {
            state = STATE_MENU;
            return;
        }
    }

    char l0[22], l1[22], l2[22];
    snprintf(l0, sizeof(l0), "Scale: %s", (pendingOp & 0x04) ? "ON" : "OFF");
    snprintf(l1, sizeof(l1), "Units:%lu", pendingUnits);
    snprintf(l2, sizeof(l2), "Range:%lu", pendingTotal);
    const char* items[] = { l0, l1, l2, "Write to Encoder", "Back" };
    char title[22];
    snprintf(title, sizeof(title), "E%d Params", encParamsWhich);
    displayMenu(title, items, nItems, encParamsSel);
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
                else if (menuSel == 2) {
                    state = STATE_ENC_PARAMS;
                    if (enc1.connected && enc2.connected) {
                        encParamsPicking = true;
                        encParamsPick = 0;
                    } else if (enc2.connected) {
                        paramsBegin(2);
                    } else {
                        paramsBegin(1);
                    }
                }
                else if (menuSel == 7) { state = STATE_CAN_DEBUG; canDebugEnter(); }
                else { state = STATE_TEST; testSel = menuSel - 3; }
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

        case STATE_ENC_PARAMS:
            encParamsUpdate(btn);
            break;

        case STATE_TEST:
            break;

        case STATE_CAN_DEBUG:
            canDebugUpdate(btn);
            break;
    }
}
