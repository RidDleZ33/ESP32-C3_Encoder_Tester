#include "canbus.h"
#include "can_debug.h"
#include <string.h>

bool canbusInit() {
    twai_general_config_t g_config = TWAI_GENERAL_CONFIG_DEFAULT(CAN_TX_PIN, CAN_RX_PIN, TWAI_MODE_NORMAL);
    twai_timing_config_t t_config = {
        .brp = 20,
        .tseg_1 = 12,
        .tseg_2 = 3,
        .sjw = 1,
        .triple_sampling = false
    };
    twai_filter_config_t f_config = TWAI_FILTER_CONFIG_ACCEPT_ALL();

    if (twai_driver_install(&g_config, &t_config, &f_config) != ESP_OK) return false;
    if (twai_start() != ESP_OK) return false;
    return true;
}

void canbusRecovery() {
    twai_status_info_t status;
    if (twai_get_status_info(&status) == ESP_OK) {
        if (status.state == TWAI_STATE_BUS_OFF) {
            Serial.println("BUS-OFF recovering");
            twai_initiate_recovery();
            delay(50);
            twai_start();
        } else if (status.state == TWAI_STATE_STOPPED) {
            Serial.println("Driver STOPPED restarting");
            twai_start();
        }
    }
}

bool canbusSend(uint32_t id, const uint8_t* data, uint8_t len) {
    twai_message_t msg = {0};
    msg.identifier = id;
    msg.extd = 0;
    msg.data_length_code = len;
    if (data && len) memcpy(msg.data, data, len);
    bool ok = twai_transmit(&msg, pdMS_TO_TICKS(10)) == ESP_OK;
    if (canDebugIsLogging()) {
        canDebugLogTx(id, data, len);
    }
    return ok;
}

bool canbusReceive(twai_message_t& frame) {
    return twai_receive(&frame, 0) == ESP_OK;
}

void canbusSendNmt(uint8_t cs, uint8_t nodeId) {
    uint8_t data[2] = {cs, nodeId};
    canbusSend(0x000, data, 2);
}

void canbusSendSync() {
    canbusSend(0x080, nullptr, 0);
}

void canbusSendSdoWrite(uint8_t nodeId, uint16_t index, uint8_t sub, uint32_t value, uint8_t size) {
    uint8_t data[8] = {0};
    data[0] = (size == 1) ? 0x2F : (size == 2) ? 0x2B : (size == 4) ? 0x23 : 0x2F;
    data[1] = index & 0xFF;
    data[2] = (index >> 8) & 0xFF;
    data[3] = sub;
    data[4] = value & 0xFF;
    data[5] = (value >> 8) & 0xFF;
    data[6] = (value >> 16) & 0xFF;
    data[7] = (value >> 24) & 0xFF;
    canbusSend(0x600 + nodeId, data, 8);
}

bool canbusSendSdoRead(uint8_t nodeId, uint16_t index, uint8_t sub, uint32_t& value, uint8_t& size) {
    uint8_t req[8] = {0x40, (uint8_t)(index & 0xFF), (uint8_t)((index >> 8) & 0xFF), sub, 0, 0, 0, 0};
    canbusSend(0x600 + nodeId, req, 8);

    uint32_t start = millis();
    while (millis() - start < 100) {
        twai_message_t rx;
        if (canbusReceive(rx) && rx.identifier == (0x580 + nodeId) && rx.data_length_code >= 4) {
            if ((rx.data[1] == (index & 0xFF)) && (rx.data[2] == ((index >> 8) & 0xFF)) && rx.data[3] == sub) {
                value = rx.data[4] | (rx.data[5] << 8) | (rx.data[6] << 16) | (rx.data[7] << 24);
                size = (rx.data[0] == 0x4F) ? 1 : (rx.data[0] == 0x4B) ? 2 : (rx.data[0] == 0x43) ? 4 : 0;
                return true;
            }
        }
        delay(1);
    }
    return false;
}
