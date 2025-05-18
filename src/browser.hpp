#pragma once

#include <Arduino.h>

#include "colors.hpp"
#include "metadata.hpp"
#include "start.hpp"
#include "input.hpp"

struct Browser {
  Credentials credentials;

  void Update() {
    tft.fillScreen(UI_BG);
    tft.setCursor(20, 20);
    tft.print("HELLO");
    delay(1000);
  }
};