#pragma once
#include <Arduino.h>

#define BUTTON 1

namespace Input {

enum Event {
  None,
  Click,
  DoubleClick,
  TripleClick,
  LongPress
};

namespace {
volatile Event lastEvent = None;
volatile bool running = false;
TaskHandle_t inputTaskHandle = nullptr;

// Settings
const unsigned long clickThreshold = 200;      // ms
const unsigned long longPressThreshold = 800;  // ms
const unsigned long multiClickTimeout = 500;   // ms

void inputTask(void*) {
  pinMode(BUTTON, INPUT_PULLDOWN);

  int clickCount = 0;
  unsigned long pressStart = 0;
  bool wasPressed = false;

  while (running) {
    bool isPressed = digitalRead(BUTTON) == HIGH;
    unsigned long now = millis();

    if (isPressed && !wasPressed) {
      // Button just pressed
      pressStart = now;
    }

    if (!isPressed && wasPressed) {
      // Button just released
      unsigned long pressDuration = now - pressStart;

      if (pressDuration >= longPressThreshold) {
        lastEvent = LongPress;
        clickCount = 0;
      } else {
        clickCount++;
        unsigned long firstClickTime = now;

        // wait for more clicks
        while (millis() - firstClickTime < multiClickTimeout) {
          if (digitalRead(BUTTON) == HIGH) {
            while (digitalRead(BUTTON) == HIGH) delay(10);  // debounce
            clickCount++;
          }
          delay(10);
        }

        switch (clickCount) {
          case 1: lastEvent = Click; break;
          case 2: lastEvent = DoubleClick; break;
          case 3: lastEvent = TripleClick; break;
          default: lastEvent = None; break;
        }
        clickCount = 0;
      }
    }

    wasPressed = isPressed;
    delay(10);
  }

  vTaskDelete(nullptr);
}
}

void start() {
  if (!running) {
    running = true;
    xTaskCreatePinnedToCore(
      inputTask,
      "InputTask",
      2048,
      nullptr,
      1,
      &inputTaskHandle,
      1);
  }
}

void stop() {
  if (running && inputTaskHandle != nullptr) {
    running = false;
    inputTaskHandle = nullptr;
  }
}

Event getLastEvent() {
  Event e = lastEvent;
  lastEvent = None;
  return e;
}

};
