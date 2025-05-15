#pragma once

#include <Arduino.h>
#include "metadata.hpp"
#include "input.hpp"
#include "colors.hpp"

// A simple on-screen QWERTY keyboard for TFT display
class OnScreenKeyboard {
public:
  OnScreenKeyboard(TFT_eSPI& display, int x, int y, int w, int h)
    : tft(display), originX(x), originY(y), width(w), height(h) {
    initLayout();
    cursorPos = { 0, 0 };
  }

  // draw the keyboard grid
  void draw() {
    tft.fillRect(originX, originY, width, height, COLOR_BG);
    int keyW = width / cols;
    int keyH = height / rows;
    for (int r = 0; r < rows; ++r) {
      for (int c = 0; c < cols; ++c) {
        int idx = r * cols + c;
        if (idx >= (int)keys.size()) continue;
        int x = originX + c * keyW;
        int y = originY + r * keyH;
        uint16_t keyColor = (cursorPos.first == r && cursorPos.second == c) ? COLOR_HIGHLIGHT : COLOR_KEYBG;
        tft.fillRect(x, y, keyW, keyH, keyColor);
        tft.drawRect(x, y, keyW, keyH, COLOR_BORDER);
        tft.setTextColor(COLOR_TEXT);
        tft.setTextDatum(MC_DATUM);
        tft.drawString(keys[idx], x + keyW / 2, y + keyH / 2);
      }
    }
  }

  // move cursor based on event
  void navigate(Input::Event ev) {
    switch (ev) {
      case Input::Click:
        selectKey();
        break;
      case Input::DoubleClick:
        backspace();
        break;
      case Input::LongPress:
        // accept / finish editing, signal by setting done flag
        done = true;
        break;
      case Input::TrippleClick:  // cycle through uppercase/lowercase
        toggleCase();
        break;
      default:
        break;
    }
  }

  bool isDone() const {
    return done;
  }
  String getText() const {
    return textBuffer;
  }

protected:
  TFT_eSPI& tft;
  int originX, originY, width, height;
  const int rows = 4;
  const int cols = 10;
  std::vector<String> keys;
  std::pair<int, int> cursorPos;

  String textBuffer;
  bool done = false;
  bool upper = false;

  void initLayout() {
    // QWERTY layout plus space and symbols
    String row0 = "QWERTYUIOP";
    String row1 = "ASDFGHJKL";
    String row2 = "ZXCVBNM,.;";
    // fill into keys
    for (char c : row0) keys.push_back(String(c));
    for (char c : row1) keys.push_back(String(c));
    keys.push_back("<");  // backspace symbol
    for (char c : row2) keys.push_back(String(c));
    keys.push_back(" ");  // space
    keys.push_back("OK");
  }

  void selectKey() {
    int idx = cursorPos.first * cols + cursorPos.second;
    if (idx < 0 || idx >= (int)keys.size()) return;
    String k = keys[idx];
    if (k == "<") backspace();
    else if (k == "OK") done = true;
    else textBuffer += (upper ? k : k.toLowerCase());
  }

  void backspace() {
    if (textBuffer.length() > 0) textBuffer.remove(textBuffer.length() - 1);
  }

  void toggleCase() {
    upper = !upper;
    // update displayed keys to match case
    for (auto& k : keys) {
      if (k.length() == 1 && isAlpha(k.charAt(0))) {
        k = upper ? k.toUpperCase() : k.toLowerCase();
      }
    }
  }

  bool isAlpha(char c) {
    return (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z');
  }
};

// blocking text read using on-screen keyboard
String readText(String placeholder = "") {
  OnScreenKeyboard keyboard(tft, 0, tft.height() / 2, tft.width(), tft.height() / 2);
  keyboard.textBuffer = placeholder;
  keyboard.draw();

  while (!keyboard.isDone()) {
    auto ev = Input::getLastEvent();
    if (ev != Input::None) {
      keyboard.navigate(ev);
      tft.fillRect(0, 0, tft.width(), tft.height() / 2, COLOR_BG);
      tft.setCursor(0, 0);
      tft.setTextColor(COLOR_TEXT);
      tft.print(keyboard.getText());
      keyboard.draw();
      delay(100);  // debounce
    }
  }
  return keyboard.getText();
}
