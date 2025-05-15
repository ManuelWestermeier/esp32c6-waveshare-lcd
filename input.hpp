#pragma once
#include <Arduino.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

namespace Input {

// Pin definition (change as needed)
#ifndef BUTTON_PIN
#define BUTTON_PIN 1
#endif

#ifndef BUTTON
#define BUTTON BUTTON_PIN
#endif

// Button event types
enum Event {
  None,
  Click,
  DoubleClick,
  TripleClick,
  LongPress
};

// Task and synchronization
static TaskHandle_t inputTaskHandle = nullptr;
static volatile bool running = false;
static portMUX_TYPE mux = portMUX_INITIALIZER_UNLOCKED;

// Last event storage (protected by portMUX)
static volatile Event lastEvent = None;

// Timing thresholds (ms)
static const uint32_t clickThreshold = 200;
static const uint32_t longPressThreshold = 800;
static const uint32_t multiClickTimeout = 500;

// Post an event in a thread-safe manner
static inline void postEvent(Event e) {
  portENTER_CRITICAL(&mux);
  lastEvent = e;
  portEXIT_CRITICAL(&mux);
}

// Task: polls button and detects events via a simple FSM
static void inputTask(void*) {
  pinMode(BUTTON_PIN, INPUT_PULLDOWN);

  bool wasPressed = false;
  uint32_t pressStart = 0;
  uint32_t lastChangeTime = 0;
  uint8_t clickCount = 0;
  enum { STATE_IDLE,
         STATE_PRESSED,
         STATE_WAIT } state = STATE_IDLE;

  while (running) {
    bool isPressed = digitalRead(BUTTON_PIN) == HIGH;
    uint32_t now = millis();

    switch (state) {
      case STATE_IDLE:
        if (isPressed) {
          pressStart = now;
          state = STATE_PRESSED;
        }
        break;

      case STATE_PRESSED:
        if (!isPressed) {
          uint32_t duration = now - pressStart;
          if (duration >= longPressThreshold) {
            postEvent(LongPress);
            clickCount = 0;
            state = STATE_IDLE;
          } else {
            // Short press -> potential click
            clickCount++;
            lastChangeTime = now;
            state = STATE_WAIT;
          }
        }
        break;

      case STATE_WAIT:
        // If a new press occurs within timeout, handle next click
        if (isPressed && (now - lastChangeTime) > clickThreshold) {
          pressStart = now;
          state = STATE_PRESSED;
        }
        // If no further press, timeout expired -> finalize click count
        else if (!isPressed && (now - lastChangeTime) >= multiClickTimeout) {
          switch (clickCount) {
            case 1: postEvent(Click); break;
            case 2: postEvent(DoubleClick); break;
            case 3: postEvent(TripleClick); break;
            default: postEvent(None); break;
          }
          clickCount = 0;
          state = STATE_IDLE;
        }
        break;
    }

    wasPressed = isPressed;
    vTaskDelay(pdMS_TO_TICKS(10));
  }

  vTaskDelete(nullptr);
}

// Start the input task
static inline void start() {
  if (!running) {
    running = true;
    xTaskCreatePinnedToCore(
      inputTask,
      "InputTask",
      2048,
      nullptr,
      1,
      &inputTaskHandle,
      tskNO_AFFINITY);
  }
}

// Stop the input task
static inline void stop() {
  if (running && inputTaskHandle != nullptr) {
    running = false;
    vTaskDelete(inputTaskHandle);
    inputTaskHandle = nullptr;
  }
}

// Retrieve and clear the last event
static inline Event getLastEvent() {
  Event e;
  portENTER_CRITICAL(&mux);
  e = lastEvent;
  lastEvent = None;
  portEXIT_CRITICAL(&mux);
  return e;
}

}  // namespace Input
