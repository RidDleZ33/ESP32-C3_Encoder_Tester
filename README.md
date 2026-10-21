# AMARC Encoder Tester

**ESP32-C3 Super Mini diagnostic tool for Baumer BFF/BFG CANopen absolute encoders.**

A compact, menu-driven tester that reads live position, pulls device identity, and runs practical diagnostic modes on one or two encoders over a 250 kbps CAN bus. Uses an external 0.96" 128x64 SSD1306 OLED.

> **Onboard OLED version:** the original ESP32-C3 OLED board variant is preserved on the [`c3-oled`](https://github.com/RidDleZ33/ESP32-C3_Encoder_Tester/tree/c3-oled) branch.

---

## Features

- **Splash screen** — “AMARC Encoder Tester” on boot
- **Live position display** — node ID and angle for up to two encoders (larger layout)
- **5-way navigation** — up / down / left / right / center (active-low)
- **Menu system** with inverted selection (up to 7 items visible)
  - **Info** — device type, resolution, error register, name (via SDO)
  - **Config** — swap encoder roles, invert E1 / E2
  - **Tests**
    - Sweep (min / max / current angle)
    - Etch-a-Sketch (two encoders as X/Y)
    - Move + Direction
    - Bus status
- **Robust CAN** — raw TWAI at 250 kbps, automatic bus-off recovery, NMT + SYNC + TPDO configuration
- **Multi-file structure** for easy extension

---

## Hardware

| Component | Notes |
|-----------|-------|
| Board | ESP32-C3 Super Mini |
| Display | External 0.96" 128x64 SSD1306 (I2C, yellow+blue or blue) |
| CAN transceiver | Genuine SN65HVD230 or TJA1051 (3.3 V) |
| Termination | 120 Ω between CAN_H and CAN_L |
| Navigation | Common 5-way joystick module (COM → GND) |
| Encoders | Baumer BFF / BFG CANopen (CiA 406), 250 kbps |

### Pinout (defaults — change in `config.h`)

| Function | GPIO | Notes |
|----------|------|-------|
| CAN TX | 4 | |
| CAN RX | 7 | |
| OLED SDA | 8 | External SSD1306 |
| OLED SCL | 9 | External SSD1306 |
| BTN UP | 0 | INPUT_PULLUP |
| BTN DOWN | 1 | |
| BTN LEFT | 2 | |
| BTN RIGHT | 3 | |
| BTN CENTER | 10 | Select / enter |

Wire the OLED VCC to 3.3 V and GND to GND. Most 0.96" modules use address 0x3C (default).

---

## Quick Start

1. Clone the repo into your Arduino sketches folder (folder name must be `ESP32-C3_Encoder_Tester`).
2. Open `ESP32-C3_Encoder_Tester.ino`.
3. Select the **ESP32-C3 Dev Module** (or Super Mini) board and the correct port.
4. Enable **USB CDC On Boot** if you need Serial output.
5. Install the **U8g2** library (provides U8x8).
6. Upload.

Power the encoder(s) and the bus. The splash appears, then the live position screen. Press **Center** to open the menu.

---

## Project Structure

```
ESP32-C3_Encoder_Tester.ino   → main setup / loop
config.h                      → pins, timing, constants
canbus.h / .cpp               → TWAI init, send/receive, SDO helpers
encoder.h / .cpp              → node tracking, configuration, info reads
display.h / .cpp              → OLED (splash, main, menu, info)
buttons.h / .cpp              → 5-way navigation (debounced)
menu.h / .cpp                 → state machine and menus
tests.h / .cpp                → diagnostic modes
```

---

## CANopen Behavior

- Baud: **250 kbps**
- On boot-up (`0x700 + NodeID`) the tester configures TPDO1 for SYNC (type 1), sets a short event timer, enables heartbeat, and sends NMT Start.
- SYNC is transmitted every 100 ms once at least one encoder is configured.
- Position arrives on TPDO1 (`0x180 + NodeID`).

---

## Extending

- New test → add a function in `tests.cpp` and a menu entry in `menu.cpp`.
- New config option → extend the config menu array and the handler.
- Different pins → edit `config.h` only.
- Additional SDO reads → use `canbusSendSdoRead()` from `encoderLoadInfo()` or a new helper.

---

## License

Personal / internal use. Adapt as needed.

---

*Built for reliable encoder diagnostics — not a toy.*
