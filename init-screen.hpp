#pragma once

#include "./metadata.hpp"

void drawStar(int x, int y, uint16_t color) {
  tft.drawPixel(x, y, color);
  tft.drawPixel(x + 1, y, color);
  tft.drawPixel(x, y + 1, color);
  tft.drawPixel(x - 1, y, color);
  tft.drawPixel(x, y - 1, color);
}

void initScreen() {
  // Draw twinkling stars
  for (int i = 0; i < 20; i++) {
    int x = random(0, tft.width());
    int y = random(0, tft.height());
    drawStar(x, y, random(0xAAAA, 0xFFFF));
    delay(30);
  }

  // Fantasy glow effect animation
  for (uint8_t index = 30; index < 80; index++) {
    tft.setTextSize(6);

    // Glow effect color cycling
    uint16_t glowColor = tft.color565(
      100 + (index * 3) % 155,
      50 + (index * 2) % 155,
      200);

    tft.setTextColor(glowColor);
    tft.setCursor(index, index);
    tft.println("MW");

    tft.setCursor(index - 10, index + 60);
    tft.println("OS");

    delay(40);
  }

  // Magical fade-out
  for (int b = 255; b > 0; b -= 5) {
    uint16_t fadeColor = tft.color565(b, b / 2, b);
    tft.fillScreen(fadeColor);
    delay(20);
  }

  tft.fillScreen(UI_BG);  // Final background
}
