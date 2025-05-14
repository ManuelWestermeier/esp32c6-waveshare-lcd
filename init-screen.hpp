#pragma once

#include "metadata.hpp"
#include "colors.hpp"

void initScreen() {
  for (uint8_t index = 30; index < 60; index++) {
    tft.setTextColor(0x07ff + (index << 11));
    tft.setTextSize(8);
    tft.setCursor(index, index);
    tft.println("MW");
    tft.setCursor(index, index + 100);
    tft.println("OS");
    delay(60);
  }
  delay(300);
  tft.fillScreen(UI_BG);
  tft.setTextColor(UI_Text);
}