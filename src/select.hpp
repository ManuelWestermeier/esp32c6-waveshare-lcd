#pragma once

#include "metadata.hpp"
#include "colors.hpp"
#include "input.hpp"
#include <vector>

int select(const std::vector<String>& options) {
  const int itemHeight = 20;
  const int screenHeight = 172;
  const int screenWidth = 320;
  const int visibleItems = screenHeight / itemHeight;

  int selectedIndex = 0;
  int scrollOffset = 0;
  int totalItems = options.size();

  while (true) {
    // Draw UI
    tft.fillScreen(UI_BG);
    tft.setTextColor(UI_Text);
    tft.setTextSize(2);

    // Draw visible items
    for (int i = 0; i < visibleItems; ++i) {
      int idx = scrollOffset + i;
      if (idx >= totalItems) break;

      int y = i * itemHeight;

      if (idx == selectedIndex) {
        tft.fillRect(0, y, screenWidth, itemHeight, UI_Secondary);
        tft.setTextColor(UI_BG);
      } else {
        tft.setTextColor(UI_Text);
      }

      tft.setCursor(4, y + 2);
      tft.print(options[idx]);
    }

    // Scroll indicators
    if (scrollOffset > 0) {
      tft.fillTriangle(screenWidth - 12, 2, screenWidth - 2, 2, screenWidth - 7, 10, UI_Text);  // Up
    }
    if (scrollOffset + visibleItems < totalItems) {
      tft.fillTriangle(screenWidth - 12, screenHeight - 2, screenWidth - 2, screenHeight - 2, screenWidth - 7, screenHeight - 10, UI_Text);  // Down
    }

    // Input handling
    auto event = Input::getLastEvent();

    if (event == Input::Click) {
      if (selectedIndex < totalItems - 1) {
        selectedIndex++;
        if (selectedIndex >= scrollOffset + visibleItems) {
          scrollOffset++;
        }
      }
    } else if (event == Input::DoubleClick) {
      if (selectedIndex > 0) {
        selectedIndex--;
        if (selectedIndex < scrollOffset) {
          scrollOffset--;
        }
      }
    } else if (event == Input::LongPress) {
      return selectedIndex;
    } else if (event == Input::TripleClick) {
      return -1;
    }

    delay(30);  // Smoothing
  }
}
