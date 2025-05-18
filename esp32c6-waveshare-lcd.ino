#include <Arduino.h>

#include "src/colors.hpp"
#include "src/metadata.hpp"
#include "src/input.hpp"
#include "src/init.hpp"
#include "src/init-screen.hpp"
#include "src/start.hpp"

// #define USE_INIT_SCREEN
// #define USE_AUTH

Credentials credentials;

void setup() {
  init();
  Input::start();
#ifdef USE_INIT_SCREEN
  initScreen();
#endif
#ifdef USE_AUTH
  credentials = start();
#else
  credentials = { "h", "h", "aaa9402664f1a41f40ebbc52c9993eb66aeb366602958fdfaa283b71e64db123", "aaa9402664f1a41f40ebbc52c9993eb66aeb366602958fdfaa283b71e64db123" };
  WiFi.begin("LocalHost", "hhhhhhhy");

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
  }
#endif
}

void loop() {
  tft.println(WiFi.localIP().toString());
}
