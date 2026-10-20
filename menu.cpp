#include "menu.h"
#include "display.h"
#include "encoder.h"

static AppState state = STATE_MAIN;
static int menuSel = 0;
static int configSel = 0;
static int testSel = 0;

static const char* mainMenu[] = {"Info", "Config", "Sweep Test", "Etch-a-Sketch", "Move+Dir", "Status"};
static const int mainMenuCount = 6;

static const char* configMenu[] = {"Swap Roles", "Invert E1", "Invert E2", "Back"};
static const int configMenuCount = 4;

void menuInit() {
    state = STATE_MAIN;
    menuSel = 0;
}

AppState menuGetState() { return state; }
void menuSetState(AppState s) { state = s; }
int menuGetTestIndex() { return testSel; }

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
                if (menuSel == 0) state = STATE_INFO;
                else if (menuSel == 1) { state = STATE_CONFIG; configSel = 0; }
                else { state = STATE_TEST; testSel = menuSel - 2; }
            }
            if (btn == BTN_LEFT) state = STATE_MAIN;
            displayMenu("Menu", mainMenu, mainMenuCount, menuSel);
            break;

        case STATE_INFO:
            displayInfo();
            if (btn == BTN_CENTER || btn == BTN_LEFT) state = STATE_MENU;
            break;

        case STATE_CONFIG:
            if (btn == BTN_UP && configSel > 0) configSel--;
            if (btn == BTN_DOWN && configSel < configMenuCount - 1) configSel++;
            if (btn == BTN_CENTER) {
                if (configSel == 0) {
                    // Swap roles: just swap the two structs
                    EncoderData tmp = enc1;
                    enc1 = enc2;
                    enc2 = tmp;
                } else if (configSel == 1) {
                    enc1.invert = !enc1.invert;
                } else if (configSel == 2) {
                    enc2.invert = !enc2.invert;
                } else if (configSel == 3) {
                    state = STATE_MENU;
                }
            }
            if (btn == BTN_LEFT) state = STATE_MENU;
            displayMenu("Config", configMenu, configMenuCount, configSel);
            break;

        case STATE_TEST:
            // Tests handle their own display and exit on CENTER or LEFT
            break;
    }
}
