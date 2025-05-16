#pragma once

#include "metadata.hpp"
#include "colors.hpp"
#include "input.hpp"
#include "read-text.hpp"
#include <vector>

int select(const std::vector<String>& originalOptions) {
  const int itemHeight = 20;
  const int screenHeight = 172;
  const int screenWidth = 320;
  const int visibleItems = screenHeight / itemHeight;

  int selectedIndex = 0;
  int scrollOffset = 0;
  std::vector<String> filteredOptions = originalOptions;

  bool filtering = false;
  String searchTerm = "";

  auto redraw = [&]() {
    tft.fillScreen(UI_BG);
    tft.setTextColor(UI_Text);
    tft.setTextSize(2);

    int totalItems = filteredOptions.size();
    int startIdx = scrollOffset;
    int endIdx = min(startIdx + visibleItems, totalItems);

    for (int i = startIdx; i < endIdx; ++i) {
      int y = (i - scrollOffset) * itemHeight;

      if (i == selectedIndex) {
        tft.fillRect(0, y, screenWidth, itemHeight, UI_Secondary);
        tft.setTextColor(UI_BG);
      } else {
        tft.setTextColor(UI_Text);
      }

      tft.setCursor(4, y + 2);
      tft.print(filteredOptions[i]);
    }

    // Scroll arrows
    if (scrollOffset > 0) {
      tft.fillTriangle(screenWidth - 12, 2, screenWidth - 2, 2, screenWidth - 7, 10, UI_Text);
    }
    if (scrollOffset + visibleItems < totalItems) {
      tft.fillTriangle(screenWidth - 12, screenHeight - 2, screenWidth - 2, screenHeight - 2, screenWidth - 7, screenHeight - 10, UI_Text);
    }
  };

  redraw();  // Initial draw

  while (true) {
    auto event = Input::getLastEvent();

    if (event == Input::None) {
      delay(10);
      continue;
    }

    int totalItems = filteredOptions.size();

    if (event == Input::Click) {
      selectedIndex++;
      if (selectedIndex >= totalItems) {
        selectedIndex = 0;
        scrollOffset = 0;
      } else if (selectedIndex >= scrollOffset + visibleItems) {
        scrollOffset++;
      }
      redraw();
    } else if (event == Input::DoubleClick) {
      selectedIndex--;
      if (selectedIndex < 0) {
        selectedIndex = totalItems - 1;
        scrollOffset = max(0, totalItems - visibleItems);
      } else if (selectedIndex < scrollOffset) {
        scrollOffset--;
      }
      redraw();
    } else if (event == Input::TripleClick) {
      return -1;
    } else if (event == Input::LongPress) {
      // Handle "Reset search"
      if (filtering && selectedIndex == 0 && filteredOptions[0] == "Reset search") {
        filteredOptions = originalOptions;
        filtering = false;
        selectedIndex = 0;
        scrollOffset = 0;
        redraw();
        continue;
      }

      return selectedIndex;
    }

    // Trigger search if list is long enough and user triple-clicks at top
    if (originalOptions.size() > 15 && event == Input::LongPress && selectedIndex == 0) {
      String input = readText("Search", searchTerm);
      input.toLowerCase();
      searchTerm = input;

      filteredOptions.clear();

      // Add reset option
      filteredOptions.push_back("Reset search");

      for (const auto& item : originalOptions) {
        String name = item;
        name.toLowerCase();

        if (name.startsWith(searchTerm)) {
          filteredOptions.push_back(item);
        }
      }

      filtering = true;
      selectedIndex = 0;
      scrollOffset = 0;
      redraw();
    }

    delay(20);
  }
}
