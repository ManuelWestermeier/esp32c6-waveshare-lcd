#pragma once

#include "metadata.hpp"
#include "colors.hpp"
#include "input.hpp"
#include "read-text.hpp"

#include <vector>

bool ok(String question);

int select(const std::vector<String>& originalOptions) {
  const int itemHeight = 31;
  const int screenHeight = 320;
  const int screenWidth = 172;
  const int visibleItems = screenHeight / itemHeight;

  std::vector<String> filteredOptions;
  bool filtering = false;
  String searchTerm = "";

  bool useSearchButton = (originalOptions.size() > 9);

  auto rebuildFiltered = [&]() {
    filteredOptions.clear();
    if (useSearchButton) filteredOptions.push_back("Search");
    if (filtering) {
      filteredOptions.push_back("Reset search");
      for (const auto& item : originalOptions) {
        String name = item;
        name.toLowerCase();
        if (name.startsWith(searchTerm)) {
          filteredOptions.push_back(item);
        }
      }
    } else {
      for (const auto& item : originalOptions) {
        filteredOptions.push_back(item);
      }
    }
  };

  rebuildFiltered();

  int selectedIndex = 0;
  int scrollOffset = 0;

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
      }

      tft.setTextColor(UI_Text);
      tft.setCursor(10, y + 10);
      if (filteredOptions[i].length() > 10)
        tft.setTextSize(1);
      tft.print(filteredOptions[i]);
    }
  };

  redraw();

  while (true) {
    auto event = Input::getLastEvent();
    if (event == Input::None) {
      delay(10);
      continue;
    }

    int totalItems = filteredOptions.size();
    if (totalItems == 0) continue;

    if (event == Input::Click) {
      selectedIndex++;
      if (selectedIndex >= totalItems) {
        selectedIndex = 0;
        scrollOffset = 0;
      } else if (selectedIndex >= scrollOffset + visibleItems) {
        scrollOffset = min(scrollOffset + 1, totalItems - visibleItems);
      }
      redraw();
    } else if (event == Input::DoubleClick) {
      selectedIndex--;
      if (selectedIndex < 0) {
        selectedIndex = totalItems - 1;
        scrollOffset = max(0, totalItems - visibleItems);
      } else if (selectedIndex < scrollOffset) {
        scrollOffset = max(0, scrollOffset - 1);
      }
      redraw();
    } else if (event == Input::TripleClick) {
      return -1;
    } else if (event == Input::LongPress) {
      // Handle Search
      if (useSearchButton && selectedIndex == 0) {
        String input = readText("Search", searchTerm);
        input.toLowerCase();

        if (input.length() > 0) {
          searchTerm = input;
          filtering = true;
        } else {
          filtering = false;
          searchTerm = "";
        }

        rebuildFiltered();
        selectedIndex = 0;
        scrollOffset = 0;
        redraw();
        continue;
      }

      // Handle Reset search
      if (filtering) {
        int resetIndex = useSearchButton ? 1 : 0;
        if (selectedIndex == resetIndex) {
          filtering = false;
          searchTerm = "";
          rebuildFiltered();
          selectedIndex = 0;
          scrollOffset = 0;
          redraw();
          continue;
        }
      }

      // Calculate index into originalOptions
      int stringIndex;
      if (useSearchButton) {
        stringIndex = filtering ? selectedIndex - 2 : selectedIndex - 1;
      } else {
        stringIndex = selectedIndex;
      }

      if (stringIndex < 0 || stringIndex >= (int)filteredOptions.size()) {
        continue;
      }

      String selectedString = filteredOptions[selectedIndex];
      int realIndex = -1;
      for (int i = 0; i < (int)originalOptions.size(); i++) {
        if (originalOptions[i] == selectedString) {
          realIndex = i;
          break;
        }
      }

      if (realIndex < 0) continue;

      if (ok("Select: " + originalOptions[realIndex] + "?")) {
        return realIndex;
      } else {
        redraw();
      }
    }

    delay(20);
  }
}
