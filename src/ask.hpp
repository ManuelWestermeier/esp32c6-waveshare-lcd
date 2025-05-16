#pragma once

#include "metadata.hpp"
#include "read-text.hpp"
#include "input.hpp"
#include "colors.hpp"

String ask(String question, String _default = "") {
  return readText(question, _default);
}

bool ok(String question) {
  tft.setTextSize(question.length() > 50 ? 1 : 2);
  tft.fillScreen(UI_BG);
  tft.setTextColor(UI_Text);

  tft.setCursor(0, 10);
  tft.println(question);

  bool focusYes = false;
  bool prevFocusYes = !focusYes;

  tft.setTextSize(3);

  // Button geometry
  int yesX = 30;
  int noX = 172 - 30 - 2 * 18;
  int yesWidth = 18 * 3 + 20;
  int noWidth = 18 * 2 + 20;
  int y = 290;
  int height = 24;

  int currentX = noX - 10;  // Initial position: NO
  int targetX = currentX;
  int currentW = noWidth;
  int targetW = currentW;

  unsigned long lastUpdate = millis();
  const int animationDuration = 200;  // ms

  while (true) {
    auto event = Input::getLastEvent();

    if (event == Input::Click) {
      focusYes = !focusYes;

      // Set new target position and width
      targetX = (focusYes ? yesX : noX) - 10;
      targetW = focusYes ? yesWidth : noWidth;
      lastUpdate = millis();
    } else if (event == Input::LongPress) {
      tft.setTextSize(2);
      return focusYes;
    }

    if (prevFocusYes == focusYes) {
      delay(10);
      continue;
    }

    // Animate movement over 200ms
    unsigned long now = millis();
    float progress = min(1.0f, (now - lastUpdate) / float(animationDuration));

    // Linear interpolation for smooth movement
    int drawX = currentX + (targetX - currentX) * progress;
    int drawW = currentW + (targetW - currentW) * progress;

    // Redraw background first
    tft.fillRect(yesX - 10, y, yesWidth, height, UI_BG);
    tft.fillRect(noX - 10, y, noWidth, height, UI_BG);

    // Draw sliding highlight
    tft.fillRect(drawX, y, drawW, height, UI_Secondary);

    // Draw text
    tft.setCursor(yesX, y);
    tft.print("YES");

    tft.setCursor(noX, y);
    tft.print("NO");

    // Update animation state
    if (progress >= 1.0f) {
      currentX = targetX;
      currentW = targetW;
    }

    delay(20);  // ~50 FPS

    prevFocusYes = focusYes;
  }
}
