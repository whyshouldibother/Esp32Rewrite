
#include "display.h"
#include "constants.h"
U8G2_SH1106_128X64_NONAME_F_HW_I2C display(U8G2_R0);

void clearSection(uint8_t x, uint8_t y, uint8_t w, uint8_t h) {
  display.setDrawColor(0);
  display.drawBox(x, y, w, h);
  display.setDrawColor(1);
}

void beginDisplay() {
  Wire.begin(display_sda, display_sdc);
  Wire.setClock(100000);

  display.begin();
  display.clearBuffer();
  display.setDrawColor(1);
  display.setFont(u8g2_font_5x8_mr);
}