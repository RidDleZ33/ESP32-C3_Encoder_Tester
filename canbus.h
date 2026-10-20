#pragma once
#include <Arduino.h>
#include "driver/twai.h"
#include "config.h"

bool canbusInit();
void canbusRecovery();
bool canbusSend(uint32_t id, const uint8_t* data, uint8_t len);
bool canbusReceive(twai_message_t& frame);
void canbusSendNmt(uint8_t cs, uint8_t nodeId);
void canbusSendSync();
void canbusSendSdoWrite(uint8_t nodeId, uint16_t index, uint8_t sub, uint32_t value, uint8_t size);
bool canbusSendSdoRead(uint8_t nodeId, uint16_t index, uint8_t sub, uint32_t& value, uint8_t& size);
