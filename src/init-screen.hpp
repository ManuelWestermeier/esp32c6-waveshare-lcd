#pragma once

#include "metadata.hpp"
#include "colors.hpp"
#include "color.hpp"
#include "rgb-led.hpp"

void initScreen() {
  tft.setTextSize(9);  // Set once, not every frame

  for (uint8_t index = 40; index < 50; index++) {
    // Interpolate between UI_Text and UI_BG
    Color interpolated = colorTransition(UI_BG, colorTransition(UI_Secondary, Color(0, 0, 0), 200), (index - 30) * 255 / 20);
    tft.setTextColor(interpolated);
    setRGB_LEDColor(interpolated);

    tft.setCursor(index - 10, index + 30);
    tft.println("MW");

    tft.setCursor(index - 10, index + 120);
    tft.println("OS");

    delay(50);
  }

  // Horizontal lines
  for (int y = 0; y < tft.height(); y += 20) {
    tft.drawLine(0, y, tft.width(), y, UI_Secondary);
    delay(50);
  }

  // Vertical lines
  for (int x = 0; x < tft.width(); x += 20) {
    tft.drawLine(x, 0, x, tft.height(), UI_Secondary);
    delay(50);
  }

  tft.fillScreen(UI_BG);
  tft.setTextColor(UI_Text);
}
