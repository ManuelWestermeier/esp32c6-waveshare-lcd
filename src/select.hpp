#pragma once

#include "metadata.hpp"
#include "colors.hpp"
#include "input.hpp"
#include "read-text.hpp"

bool ok(String question);

#include <vector>

int select(const std::vector<String>& originalOptions) {
  const int itemHeight = 30;
  const int screenHeight = 320;
  const int screenWidth = 172;
  const int visibleItems = screenHeight / itemHeight;

  std::vector<String> filteredOptions;
  bool filtering = false;
  String searchTerm = "";

  bool useSearchButton = (originalOptions.size() > 5);

  // Build the filteredOptions initially
  auto rebuildFiltered = [&]() {
    filteredOptions.clear();
    if (useSearchButton) {
      filteredOptions.push_back("Search");
    }
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
        tft.setTextColor(UI_Text);
      } else {
        tft.setTextColor(UI_Text);
      }

      tft.setCursor(10, y + 10);
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
      // Handle special buttons when filtering or search button
      if (useSearchButton && selectedIndex == 0) {
        // User clicked "Search" button → ask for search term
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

      if (filtering) {
        // Reset search option is always at index 1 when filtering is active and search button shown
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

      // Normal selection
      int realIndex;

      if (useSearchButton) {
        if (filtering) {
          // filteredOptions: ["Search", "Reset search", ...filtered items...]
          realIndex = selectedIndex - 2;
        } else {
          // filteredOptions: ["Search", ...all items...]
          realIndex = selectedIndex - 1;
        }
      } else {
        // No search button
        realIndex = selectedIndex;
      }

      // If invalid selection or search/reset option clicked as actual selection, ignore
      if (realIndex < 0 || realIndex >= (int)originalOptions.size()) {
        // Do nothing, continue selection
        continue;
      }

      // Confirm selection with user
      bool confirmed = ok("Select: " + originalOptions[realIndex] + "?");
      if (confirmed) {
        return realIndex;
      } else {
        redraw();
        continue;  // back to selection
      }
    }

    delay(20);
  }
}
