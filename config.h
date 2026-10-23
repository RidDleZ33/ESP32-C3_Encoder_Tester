#pragma once

// ========== Hardware Pins ==========
// CAN (confirmed working)
#define CAN_TX_PIN          GPIO_NUM_4
#define CAN_RX_PIN          GPIO_NUM_7

// OLED (onboard I2C)
#define PIN_SDA             5
#define PIN_SCL             6

// 5-way navigation (COM to GND, active low, INPUT_PULLUP)
// Change these if you wire differently. Avoid 4,5,6,7.
#define BTN_UP_PIN          0
#define BTN_DOWN_PIN        1
#define BTN_LEFT_PIN        2
#define BTN_RIGHT_PIN       3
#define BTN_CENTER_PIN      10

// ========== Encoder ==========
const uint16_t ENCODER_MAX_STEPS = 4096;  // 12-bit singleturn (180° physical was mapping to 360° with 8192)
const uint32_t ENCODER_TIMEOUT_MS = 4000;

// ========== Timing ==========
const uint32_t SPLASH_MS = 2000;
const uint32_t SYNC_INTERVAL_MS = 100;
const uint32_t UI_UPDATE_MS = 100;
const uint32_t BUTTON_DEBOUNCE_MS = 50;

// ========== Display ==========
const uint8_t NORMAL_CONTRAST = 180;
