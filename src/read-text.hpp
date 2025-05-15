#pragma once
#include <Arduino.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ST7789.h>
#include "input.hpp"
#include "colors.hpp"

// Simple instructions: Click=←, Double=→, Triple=↑, Long=Select
static const char* keyboardDesc =
  "Click=Left  Double=Right  Triple=Up  Long=Select";

// 4×10 keys: letters, digits, Backspace '<', OK
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
    : x(x + pad), y(y + pad), w(w - 2 * pad), h(h - 2 * pad), cursor(0), done(false) {
    // keyW = w / (cols + 2);
    // keyH = h / (rows + 2);
    keyW = 15;
    keyH = 20;
  }

  void setText(const String& s) {
    text = s;
  }

  void drawKey(int idx) {
    if (idx < 0 || idx >= cols * rows) return;
    int r = idx / cols;
    int c = idx % cols;
    int px = x + c * keyW;
    int py = y + r * keyH;
    bool sel = (idx == cursor);
    // key box
    tft.fillRect(px + 1, py + 1, keyW - 2, keyH - 2, sel ? UI_Secondary : UI_BG);
    // label
    String L = keyLabels[idx];
    if (L.length() > 0) {
      tft.setTextSize(1);
      tft.setTextColor(UI_Text);
      if (L == "OK" || L == "<") {
        tft.setCursor(px + 4, py + keyH / 2 - 6);
        tft.print(L);
      } else {
        char ch = L.charAt(0);
        char d = isAlpha(ch) ? char(tolower(ch)) : ch;
        tft.setCursor(px + keyW / 2 - 4, py + keyH / 2 - 6);
        tft.print(d);
      }
    }
  }

  void drawAll() {
    // background for keyboard
    tft.fillRect(x, y, w, h, UI_BG);
    for (int i = 0; i < cols * rows; i++) {
      drawKey(i);
    }
  }

  void navigate(Input::Event ev) {
    switch (ev) {
      case Input::Click:  // left
        cursor = (cursor % cols == cols - 1) ? cursor - (cols - 1) : cursor + 1;
        break;
      case Input::DoubleClick:  // up
        cursor = (cursor > cols) ? cursor + cols * (rows + 1) : cursor + cols;
        break;
      default:
        break;
    }
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
  static constexpr int cols = 10;
  static constexpr int rows = 4;
  int x, y, w, h;
  int keyW, keyH;
  int cursor;
  bool done;
  String text;

  bool isAlpha(char c) {
    return (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z');
  }
};

// Blocking read: fits 172×320 vertical
inline String readText(const String& placeholder = "") {
  const int W = tft.width();
  const int H = tft.height();
  const int headerH = 20;
  const int keyboardH = H * 0.6;  // 60% of screen height for keyboard

  // Draw header and input area
  auto drawHeader = [&](const String& currentText) {
    // Description line
    tft.fillRect(0, 0, W, headerH, UI_BG);
    tft.setCursor(20, 5);
    tft.setTextSize(1);
    tft.setTextColor(UI_Text);
    tft.print(keyboardDesc);
    // Input text area
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

  return kb.getText();
}
