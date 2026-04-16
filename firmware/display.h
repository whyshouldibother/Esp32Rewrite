#ifndef DISPLAY_H
#define DISPLAY_H
#include <U8g2lib.h>
#include <Wire.h>

extern U8G2_SH1106_128X64_NONAME_F_HW_I2C display;
void clearSection(uint8_t,uint8_t,uint8_t,uint8_t);
void beginDisplay();
#endif