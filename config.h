#pragma once

// ========== Hardware Pins ==========
// CAN (confirmed working)
#define CAN_TX_PIN          GPIO_NUM_4
#define CAN_RX_PIN          GPIO_NUM_7

// External 0.96" SSD1306 OLED (I2C)
// Change these to match your wiring on the Super Mini
#define PIN_SDA             8
#define PIN_SCL             9

// 5-way navigation (COM to GND, active low, INPUT_PULLUP)
// Avoid CAN pins (4/7) and OLED pins (8/9)
#define BTN_UP_PIN          0
#define BTN_DOWN_PIN        1
#define BTN_LEFT_PIN        2
#define BTN_RIGHT_PIN       3
#define BTN_CENTER_PIN      10

// ========== Encoder ==========
const uint16_t ENCODER_MAX_STEPS = 8192;  // 13-bit singleturn default
const uint32_t ENCODER_TIMEOUT_MS = 4000;

// ========== Timing ==========
const uint32_t SPLASH_MS = 2000;
const uint32_t SYNC_INTERVAL_MS = 100;
const uint32_t UI_UPDATE_MS = 100;
const uint32_t BUTTON_DEBOUNCE_MS = 50;

// ========== Display ==========
const uint8_t NORMAL_CONTRAST = 180;
// 128x64 = 16 columns x 8 rows in U8X8
const uint8_t OLED_COLS = 16;
const uint8_t OLED_ROWS = 8;
