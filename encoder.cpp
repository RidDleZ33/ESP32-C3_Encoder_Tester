#include "encoder.h"
#include "canbus.h"
#include <stdio.h>

EncoderData enc1;
EncoderData enc2;

void encoderInit() {
    enc1 = EncoderData();
    enc2 = EncoderData();
}

uint8_t extractNodeId(uint32_t cobId) {
    if (cobId >= 0x181 && cobId <= 0x1FF) return cobId - 0x180;
    if (cobId >= 0x701 && cobId <= 0x7FF) return cobId - 0x700;
    if (cobId >= 0x581 && cobId <= 0x5FF) return cobId - 0x580;
    return 0;
}

void encoderConfigure(uint8_t nodeId) {
    // TPDO1 transmission type = 1 (every SYNC)
    canbusSendSdoWrite(nodeId, 0x1800, 0x02, 1, 1);
    delay(20);
    // Event timer 50 ms fallback
    canbusSendSdoWrite(nodeId, 0x1800, 0x05, 50, 2);
    delay(20);
    // Heartbeat 1000 ms
    canbusSendSdoWrite(nodeId, 0x1017, 0x00, 1000, 2);
    delay(20);
    // NMT Start
    canbusSendNmt(0x01, nodeId);
}

void encoderLoadInfo(EncoderData& enc) {
    if (!enc.connected || enc.nodeId == 0) return;
    uint32_t val;
    uint8_t sz;

    if (canbusSendSdoRead(enc.nodeId, 0x1000, 0x00, val, sz)) enc.deviceType = val;
    if (canbusSendSdoRead(enc.nodeId, 0x1001, 0x00, val, sz)) enc.errorReg = val & 0xFF;
    if (canbusSendSdoRead(enc.nodeId, 0x1018, 0x01, val, sz)) enc.vendorId = val;
    if (canbusSendSdoRead(enc.nodeId, 0x1018, 0x02, val, sz)) enc.productCode = val;
    if (canbusSendSdoRead(enc.nodeId, 0x1018, 0x04, val, sz)) enc.serial = val;
    if (canbusSendSdoRead(enc.nodeId, 0x6001, 0x00, val, sz)) enc.resolution = val;

    // Device name (string) - simplified: read first 4 bytes as fallback
    if (canbusSendSdoRead(enc.nodeId, 0x1008, 0x00, val, sz)) {
        snprintf(enc.deviceName, sizeof(enc.deviceName), "%c%c%c%c",
                 (char)(val & 0xFF), (char)((val >> 8) & 0xFF),
                 (char)((val >> 16) & 0xFF), (char)((val >> 24) & 0xFF));
    }
    enc.infoLoaded = true;
}

void assignOrUpdate(uint8_t nodeId) {
    if (nodeId == 0) return;
    EncoderData* target = nullptr;
    if (enc1.connected && enc1.nodeId == nodeId) target = &enc1;
    else if (enc2.connected && enc2.nodeId == nodeId) target = &enc2;
    else if (!enc1.connected) target = &enc1;
    else if (!enc2.connected) target = &enc2;

    if (target) {
        bool wasNew = !target->connected;
        target->nodeId = nodeId;
        if (wasNew) {
            target->connectedAtMs = millis();
            target->connected = true;
            if (!target->configured) {
                Serial.printf("Configuring node %d\n", nodeId);
                encoderConfigure(nodeId);
                target->configured = true;
            }
        }
        target->lastFrameMs = millis();
    }
}

void encoderProcessFrame(const twai_message_t& frame) {
    if (frame.identifier == 0x000 || frame.identifier == 0x080) return;
    uint8_t nodeId = extractNodeId(frame.identifier);
    if (nodeId == 0) return;

    assignOrUpdate(nodeId);

    EncoderData* enc = nullptr;
    if (enc1.connected && enc1.nodeId == nodeId) enc = &enc1;
    else if (enc2.connected && enc2.nodeId == nodeId) enc = &enc2;
    if (!enc) return;

    if ((frame.identifier & 0x780) == 0x180 && frame.data_length_code >= 2) {
        uint32_t pos = frame.data[0] | (frame.data[1] << 8);
        if (frame.data_length_code >= 4) {
            pos |= ((uint32_t)frame.data[2] << 16) | ((uint32_t)frame.data[3] << 24);
        }
        enc->rawPosition = pos;
        uint32_t steps = enc->resolution ? enc->resolution : ENCODER_MAX_STEPS;
        uint32_t single = pos % steps;
        enc->angleDegrees = (single * 360.0f) / steps;
        if (enc->invert) enc->angleDegrees = 360.0f - enc->angleDegrees;
        enc->lastFrameMs = millis();
    } else if ((frame.identifier & 0x700) == 0x700) {
        enc->lastFrameMs = millis();
    }
}

void encoderTimeoutCheck() {
    uint32_t now = millis();
    if (enc1.connected && (now - enc1.lastFrameMs > ENCODER_TIMEOUT_MS)) {
        enc1.connected = false;
        enc1.nodeId = 0;
        enc1.configured = false;
        enc1.infoLoaded = false;
    }
    if (enc2.connected && (now - enc2.lastFrameMs > ENCODER_TIMEOUT_MS)) {
        enc2.connected = false;
        enc2.nodeId = 0;
        enc2.configured = false;
        enc2.infoLoaded = false;
    }
}

EncoderData* encoderByNode(uint8_t nodeId) {
    if (enc1.connected && enc1.nodeId == nodeId) return &enc1;
    if (enc2.connected && enc2.nodeId == nodeId) return &enc2;
    return nullptr;
}

float encoderAngle(const EncoderData& enc) {
    return enc.angleDegrees;
}
