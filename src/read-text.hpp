#pragma once
#include <Arduino.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ST7789.h>
#include "input.hpp"
#include "colors.hpp"

// Simple instructions: Click=←, Double=→, Triple=↑, Long=Select
static const char* keyboardDesc = "Click=Right Double=Up\nTriple=Down Long=Select";

// 42 Labels (letters, digits, Backspace '<', OK)
static const String keyLabels[] = {
  "Q", "W", "E", "R", "T", "Y", "U", "I", "O", "P",
  "A", "S", "D", "F", "G", "H", "J", "K", "L", ";",
  "Z", "X", "C", "V", "B", "N", "M", ",", ".", "?",
  "1", "2", "3", "4", "5", "6", "7", "8", "9", "0",
  "<", "OK"
};

class OnScreenKeyboard {
public:
  OnScreenKeyboard(int x, int y, int w, int h, int pad = 5)
    : x(x + pad), y(y + pad),
      w(w - 2 * pad), h(h - 2 * pad),
      cursor(0), done(false) {
    keyCount = sizeof(keyLabels) / sizeof(keyLabels[0]);
    cols = 10;
    rows = (keyCount + cols - 1) / cols;  // calculate rows based on count
    keyW = w / cols;
    keyH = h / rows;
  }

  void setText(const String& s) {
    text = s;
  }

  void drawKey(int idx) {
    if (idx < 0 || idx >= keyCount) return;
    int r = idx / cols;
    int c = idx % cols;
    int px = x + c * keyW;
    int py = y + r * keyH;
    bool sel = (idx == cursor);
    // draw key background
    tft.fillRect(px + 1, py + 1, keyW - 2, keyH - 2, sel ? UI_Secondary : UI_BG);
    // draw label
    String L = keyLabels[idx];
    if (L.length() == 0) return;
    tft.setTextSize(1);
    tft.setTextColor(UI_Text);
    int16_t x1, y1;
    uint16_t w1, h1;
    // center text
    tft.getTextBounds(L, px + keyW / 2, py + keyH / 2 - 4, &x1, &y1, &w1, &h1);
    tft.setCursor(px + keyW / 2 - w1 / 2, py + keyH / 2 - h1 / 2);
    tft.print(L);
  }

  void drawAll() {
    // clear keyboard area
    tft.fillRect(x, y, w, h, UI_BG);
    for (int i = 0; i < keyCount; ++i) {
      drawKey(i);
    }
  }

  void navigate(Input::Event ev) {
    int r = cursor / cols;
    int c = cursor % cols;
    switch (ev) {
      case Input::Click:  // →
        c = (c < cols - 1 ? c + 1 : 0);
        break;
      case Input::DoubleClick:  // ↑
        r = (r > 0 ? r - 1 : rows - 1);
        break;
      case Input::TripleClick:  // ↓
        r = (r < rows - 1 ? r + 1 : 0);
        break;
      default:
        return;
    }
    int newIdx = r * cols + c;
    if (newIdx >= keyCount) newIdx = keyCount - 1;
    cursor = newIdx;
  }

  void selectKey() {
    String L = keyLabels[cursor];
    if (L == "<") {
      if (text.length() > 0) text.remove(text.length() - 1);
    } else if (L == "OK") {
      done = true;
    } else if (L.length() > 0) {
      char ch = L.charAt(0);
      text += isAlpha(ch) ? char(tolower(ch)) : ch;
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
  int cursor;
  bool done;
  String text;

  bool isAlpha(char c) {
    return (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z');
  }
};

// Blocking read: fits 172×320 vertical
inline String readText(const String& placeholder = "") {
  tft.fillScreen(UI_BG);

  const int W = tft.width();
  const int H = tft.height();
  const int headerH = 20;
  const int keyboardH = H * 0.6;  // 60% of screen for keyboard

  // draw header and input area
  auto drawHeader = [&](const String& currentText) {
    tft.fillRect(0, 0, W, headerH, UI_BG);
    tft.setCursor(20, 5);
    tft.setTextSize(1);
    tft.setTextColor(UI_Text);
    tft.print(keyboardDesc);
    int textAreaY = headerH;
    int textAreaH = H - keyboardH - headerH;
    tft.fillRect(0, textAreaY, W, textAreaH, UI_BG);
    tft.setCursor(5, textAreaY + 5);
    tft.setTextSize(2);
    tft.setTextColor(UI_Text);
    tft.print(currentText);
  };

  OnScreenKeyboard kb(0, H - keyboardH, W, keyboardH, 4);
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

  auto text = kb.getText();
  
  tft.setTextSize(2);
  tft.println(text);

  delay(1500);
  tft.fillScreen(UI_BG);

  return text;
}
