#pragma once
#include <Arduino.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ST7789.h>
#include "input.hpp"
#include "colors.hpp"

// 4×10 keys, last two are Backspace and OK
static const String keyLabels[] = {
  "Q", "W", "E", "R", "T", "Y", "U", "I", "O", "P",
  "A", "S", "D", "F", "G", "H", "J", "K", "L", ";",
  "Z", "X", "C", "V", "B", "N", "M", ",", ".", "?",
  "1", "2", "3", "4", "5", "6", "7", "8", "9", "0",
  "<", "OK", "", "", "", "", "", "", "", ""
};

class OnScreenKeyboard {
public:
  OnScreenKeyboard(int x, int y, int w, int h)
    : x(x), y(y), w(w), h(h), cursor(0), upper(false) {
    keyW = w / cols;
    keyH = h / rows;
  }

  void setText(const String& s) {
    text = s;
  }

  void draw() {
    // keyboard bg
    tft.fillRect(x, y, w, h, UI_BG);

    for (int r = 0; r < rows; ++r) {
      for (int c = 0; c < cols; ++c) {
        int idx = r * cols + c;
        if (idx >= sizeof(keyLabels)) break;

        int px = x + c * keyW;
        int py = y + r * keyH;
        bool sel = (idx == cursor);

        // key box
        tft.fillRect(px + 1, py + 1, keyW - 2, keyH - 2, sel ? UI_Primary : UI_Secondary);
        tft.drawRect(px + 1, py + 1, keyW - 2, keyH - 2, UI_Text);

        // label
        String L = keyLabels[idx];
        if (L.length()) {
          char ch = L.charAt(0);
          // if "OK" or "<", use as‐is
          if (L == "OK" || L == "<") {
            tft.setCursor(px + 4, py + keyH / 2 - 6);
            tft.setTextSize(1);
            tft.setTextColor(UI_Text);
            tft.print(L);
          } else {
            // single‐char letter/digit
            char disp = upper && isAlpha(ch) ? char(toupper(ch)) : char(tolower(ch));
            tft.setCursor(px + (keyW / 2) - 4, py + (keyH / 2) - 6);
            tft.setTextSize(1);
            tft.setTextColor(UI_Text);
            tft.print(disp);
          }
        }
      }
    }
  }

  // Map the 4 gestures to cursor movement:
  //   Click → right
  //   DoubleClick → down
  //   TripleClick → left
  //   LongPress → up
  void navigate(Input::Event ev) {
    if (ev == Input::Click && cursor % cols < cols - 1) ++cursor;
    else if (ev == Input::DoubleClick && cursor / cols < rows - 1) cursor += cols;
    else if (ev == Input::TripleClick && cursor % cols > 0) --cursor;
    else if (ev == Input::LongPress && cursor / cols > 0) cursor -= cols;

    // bounds
    if (cursor < 0) cursor = 0;
    if (cursor >= cols * rows) cursor = cols * rows - 1;
  }

  // Called when user wants to *select* the current key:
  void selectKey() {
    String L = keyLabels[cursor];
    if (L == "<") {
      if (text.length()) text.remove(text.length() - 1);
    } else if (L == "OK") {
      done = true;
    } else if (L.length()) {
      char ch = L.charAt(0);
      text += (upper && isAlpha(ch)) ? char(toupper(ch)) : char(tolower(ch));
    }
  }

  bool isDone() const {
    return done;
  }
  String getText() const {
    return text;
  }

private:
  static constexpr int cols = 10;
  static constexpr int rows = 4;

  int x, y, w, h;
  int keyW, keyH;
  int cursor;
  bool upper;
  bool done = false;
  String text;

  bool isAlpha(char c) {
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
  }
};


// ——— Blocking read ———
// placeholder drawn as initial text; keyboard uses the global `tft`.
inline String readText(const String& placeholder = "") {
  const int W = tft.width(), H = tft.height();
  const int topH = H / 2;

  // draw placeholder
  tft.fillRect(0, 0, W, topH, UI_BG);
  tft.setCursor(0, 0);
  tft.setTextColor(UI_Text);
  tft.setTextSize(2);
  tft.print(placeholder);

  OnScreenKeyboard kb(0, topH, W, H - topH);
  kb.setText(placeholder);
  kb.draw();

  while (!kb.isDone()) {
    Input::Event ev = Input::getLastEvent();

    if (ev == Input::Click || ev == Input::DoubleClick || ev == Input::TripleClick || ev == Input::LongPress) {
      // first navigation
      kb.navigate(ev);

      // hold *another* DoubleClick as "select"
      if (ev == Input::DoubleClick) {
        kb.selectKey();
      }
    }

    // update display
    tft.fillRect(0, 0, W, topH, UI_BG);
    tft.setCursor(0, 0);
    tft.print(kb.getText());
    kb.draw();
    delay(100);
  }

  return kb.getText();
}
