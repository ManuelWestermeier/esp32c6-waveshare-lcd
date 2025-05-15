#include "src/colors.hpp"
#include "src/metadata.hpp"
#include "src/input.hpp"
#include "src/init.hpp"
#include "src/init-screen.hpp"

#define USE_INIT_SCREEN true

void setup() {
  init();
  Input::start();
#if USE_INIT_SCREEN
  initScreen();
#endif
}

#include "src/read-text.hpp"

void loop() {
  tft.fillScreen(UI_BG);
  tft.setCursor(20, 20);
  tft.println(readText("Hello"));

  delay(5000);
}