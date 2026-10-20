#pragma once
#include <U8x8lib.h>

extern U8X8_SSD1306_128X64_NONAME_HW_I2C u8x8;

void displayInit();
void displaySplash();
void displayMain();
void displayMenu(const char* title, const char* items[], int count, int selected);
void displayInfo();
void displayMessage(const char* line1, const char* line2 = nullptr);
