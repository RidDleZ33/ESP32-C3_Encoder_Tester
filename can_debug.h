#pragma once
#include "buttons.h"
#include "driver/twai.h"

void canDebugEnter();
void canDebugUpdate(Button btn);
bool canDebugIsLogging();
void canDebugLogTx(uint32_t id, const uint8_t* data, uint8_t len);
void canDebugLogRx(const twai_message_t& frame);
