#pragma once
#include <U8g2lib.h>

extern U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2;

void displayInit();
void displaySplash();
void displayMain();
void displayMenu(const char* title, const char* items[], int count, int selected);
void displayInfo();
void displayMessage(const char* line1, const char* line2 = nullptr);
