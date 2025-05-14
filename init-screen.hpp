#pragma once

#include "./metadata.hpp"

void initScreen() {
  for (uint8_t index = 30; index < 60; index++) {
    tft.setTextSize(8);
    tft.setCursor(index, index);
    tft.println("MW");
    tft.setCursor(index, index + 100);
    tft.println("OS");
    delay(50);
  }
  tft.fillScreen(UI_BG);
}