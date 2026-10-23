#pragma once
#include <Arduino.h>
#include "driver/twai.h"
#include "config.h"

struct EncoderData {
    uint8_t nodeId = 0;
    uint32_t rawPosition = 0;      // full multiturn absolute from TPDO
    uint32_t singleTurn = 0;       // single-turn portion (lower bits / mod resolution)
    float angleDegrees = 0.0f;
    uint32_t lastFrameMs = 0;
    uint32_t connectedAtMs = 0;
    bool connected = false;
    bool configured = false;
    bool invert = false;

    // Info cache
    char deviceName[32] = {0};
    uint32_t deviceType = 0;
    uint32_t vendorId = 0;
    uint32_t productCode = 0;
    uint32_t serial = 0;
    uint32_t resolution = 0;       // steps per revolution (0x6001 or 0x6501)
    uint8_t errorReg = 0;
    bool infoLoaded = false;
};

extern EncoderData enc1;
extern EncoderData enc2;

void encoderInit();
void encoderProcessFrame(const twai_message_t& frame);
void encoderConfigure(uint8_t nodeId);
void encoderLoadInfo(EncoderData& enc);
void encoderTimeoutCheck();
EncoderData* encoderByNode(uint8_t nodeId);
float encoderAngle(const EncoderData& enc);
