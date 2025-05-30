#pragma once

#include <Arduino.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ST7789.h>
#include "input.hpp"
#include "colors.hpp"

// 42 Labels (letters, digits, Backspace '<', Space ' ', Toggle '^', OK)
static const String keyLabels[] = {
  "1", "2", "3", "4", "5", "6", "7", "8", "9", "0",
  "Q", "W", "E", "R", "T", "Y", "U", "I", "O", "P",
  "A", "S", "D", "F", "G", "H", "J", "K", "L", ";",
  "Z", "X", "C", "V", "B", "N", "M", ",", ".", "?",
  "<", " ", "\n", "^", "OK"
};

class OnScreenKeyboard {
public:
  OnScreenKeyboard(int x, int y, int w, int h, int pad = 5, String _keyboardDesc = "   Click=Right Double=Up\n   Triple=Left Long=Select")
    : x(x + pad), y(y + pad),
      w(w - 2 * pad), h(h - 2 * pad),
      cursor(25), done(false), uppercase(false), keyboardDesc(_keyboardDesc) {
    keyCount = sizeof(keyLabels) / sizeof(keyLabels[0]);
    cols = 10;
    rows = (keyCount + cols - 1) / cols;
    keyW = (w / cols) - 1;
    keyH = (h / rows) - 15;
  }

  void setText(const String &s) {
    text = s;
  }

  void drawKey(int idx) {
    if (idx < 0 || idx >= keyCount)
      return;

    int r = idx / cols;
    int c = idx % cols;
    int px = x + c * keyW;
    int py = y + r * keyH;
    bool sel = (idx == cursor);

    // draw key background
    tft.fillRect(px - 1, py + 1, keyW, keyH, sel ? UI_Secondary : UI_BG);

    // get label and adjust for uppercase/lowercase
    String L = keyLabels[idx];
    if (L.length() == 1 && isAlpha(L.charAt(0))) {
      char base = L.charAt(0);
      L = uppercase ? String(base) : String(char(tolower(base)));
    }

    // draw label
    tft.setTextColor(UI_Text);
    tft.setTextSize(idx > 39 ? 1 : 2);

    int16_t x1, y1;
    uint16_t w1, h1;
    tft.getTextBounds(L, px + keyW / 2, py + keyH / 2 - 4, &x1, &y1, &w1, &h1);
    tft.setCursor(px + keyW / 2 - w1 / 2, py + keyH / 2 - h1 / 2);
    tft.print(L == " " ? String("_") : (L == "\n" ? String("\\") : L));
  }

  void drawAll() {
    tft.fillRect(x, y, w, h, UI_BG);
    for (int i = 0; i < keyCount; ++i)
      drawKey(i);
    tft.setTextColor(UI_Text);
  }

  void navigate(Input::Event ev) {
    int r = cursor / cols;
    int c = cursor % cols;

    // determine columns in current row (handles last row shorter length)
    int colsInRow = min(cols, keyCount - r * cols);

    if (ev == Input::Click) {  // move right
      c = (c + 1) % colsInRow;
    } else if (ev == Input::TripleClick) {  // move left
      c = (c + colsInRow - 1) % colsInRow;
    } else if (ev == Input::DoubleClick) {  // move up
      if (r > 0)
        r--;
      else
        r = rows - 1;
      // clamp to existing keys
      c = min(c, min(cols, keyCount - r * cols) - 1);
    } else {
      return;
    }

    cursor = r * cols + c;
  }

  void selectKey() {
    String L = keyLabels[cursor];
    if (L == "<") {
      if (text.length())
        text.remove(text.length() - 1);
    } else if (L == "OK") {
      done = true;
    } else if (L == "^") {
      uppercase = !uppercase;
    } else {
      char ch = L.charAt(0);
      if (isAlpha(ch))
        text += uppercase ? ch : char(tolower(ch));
      else
        text += ch;
    }
  }

  bool isDone() const {
    return done;
  }
  String getText() const {
    return text;
  }

private:
  int x, y, w, h;
  int cols, rows, keyW, keyH;
  int keyCount;
  int cursor = 25;
  bool done;
  bool uppercase;
  String text;
  String keyboardDesc;

  bool isAlpha(char c) {
    return (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z');
  }
};

inline String readText(String keyboardDesc = "   Click=Right Double=Up\n   Triple=Left Long=Select", const String &placeholder = "") {
  tft.fillScreen(UI_BG);
  const int W = tft.width();
  const int H = tft.height();
  const int headerH = 20;
  const int keyboardH = H * 0.6;

  auto drawHeader = [&](const String &cur) {
    tft.fillRect(0, 0, W, headerH, UI_BG);
    tft.setCursor(10, 5);
    tft.setTextSize(1);
    tft.setTextColor(UI_Text);
    tft.print(keyboardDesc);
    tft.fillRect(0, headerH, W, H - keyboardH - headerH, UI_BG);
    tft.setCursor(5, headerH + 5);
    tft.setTextSize(2);
    tft.print(cur);
  };

  OnScreenKeyboard kb(2, (H - keyboardH) * 1.5, W, keyboardH, 4, keyboardDesc);
  kb.setText(placeholder);
  drawHeader(placeholder);
  kb.drawAll();

  while (!kb.isDone()) {
    Input::Event ev = Input::getLastEvent();
    if (ev == Input::Click || ev == Input::DoubleClick || ev == Input::TripleClick) {
      kb.navigate(ev);
      kb.drawAll();
    } else if (ev == Input::LongPress) {
      kb.selectKey();
      drawHeader(kb.getText());
      kb.drawAll();
    }
    delay(100);
  }

  tft.fillScreen(UI_BG);
  tft.setCursor(20, 20);
  tft.setTextSize(2);
  tft.println(kb.getText());
  delay(1500);
  tft.fillScreen(UI_BG);
  return kb.getText();
}
