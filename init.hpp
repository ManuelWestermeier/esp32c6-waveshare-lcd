#pragma once

#include "metadata.hpp"
#include "colors.hpp"

void init() {
  pinMode(BUTTON, INPUT_PULLDOWN);
  // SPI init (optional—Adafruit does it for you)
  SPI.begin(TFT_SCLK, /*MISO*/ -1, TFT_MOSI);

  // Initialize display, width=172, height=320
  tft.init(172, 320);
  tft.setRotation(2);  // match your orientation
  tft.fillScreen(UI_BG);

  // backlight on
  ledcAttach(TFT_BL, 1000, 10);
  ledcWrite(TFT_BL, 255);

  // text style
  tft.setTextColor(UI_Text);
  tft.setTextSize(2);  // 2× scale
}