#pragma once

#include "metadata.hpp"
#include "colors.hpp"
#include "rgb-led.hpp"

void init()
{
  Serial.begin(115200);
  pinMode(BUTTON, INPUT_PULLDOWN);
  pixels.begin();
  // SPI init (optional—Adafruit does it for you)
  SPI.begin(TFT_SCLK, /*MISO*/ -1, TFT_MOSI);

  // Initialize display, width=172, height=320
  tft.init(172, 320);
  tft.setRotation(2); // vertical
  tft.fillScreen(UI_BG);

  // backlight on
  ledcAttach(TFT_BL, 1000, 10);
  ledcWrite(TFT_BL, 255);

  // text style
  tft.setTextColor(UI_Text);
  tft.setTextSize(2); // 2× scale
}