#pragma once
#include "buttons.h"

enum AppState {
    STATE_MAIN,
    STATE_MENU,
    STATE_INFO,
    STATE_CONFIG,
    STATE_ENC_PARAMS,
    STATE_TEST,
    STATE_CAN_DEBUG
};

void menuInit();
void menuUpdate(Button btn);
AppState menuGetState();
void menuSetState(AppState s);
int menuGetTestIndex();
