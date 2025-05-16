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

  tft.setTextSize(3);

  while (true) {
    auto event = Input::getLastEvent();

    if (event == Input::Click) {
      focusYes = !focusYes;
    } else if (event == Input::LongPress) {
      tft.setTextSize(2);
      return focusYes;
    }

    tft.fillRect((focusYes ? 30 : (172 - 30 - 2 * 18)) - 10, 290, (18 * (focusYes ? 3 : 2)) + 20, 24, UI_Secondary);

    tft.setCursor(30, 290);
    tft.print("YES");

    tft.setCursor(172 - 30 - 2 * 18, 290);
    tft.print("NO");

    delay(50);
  }
}
