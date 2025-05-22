#include <Arduino.h>

#include "wifi-secrets.hpp"

#include "src/colors.hpp"
#include "src/metadata.hpp"
#include "src/input.hpp"
#include "src/init.hpp"
#include "src/init-screen.hpp"
#include "src/browser.hpp"

// #define USE_INIT_SCREEN
// #define USE_AUTH

Browser browser;

void setup()
{
  init();
  Input::start();
#ifdef USE_INIT_SCREEN
  initScreen();
#endif
#ifdef USE_AUTH
  browser.credentials = start();
#else
  browser.credentials = {"h", "h", "aaa9402664f1a41f40ebbc52c9993eb66aeb366602958fdfaa283b71e64db123", "aaa9402664f1a41f40ebbc52c9993eb66aeb366602958fdfaa283b71e64db123"};
  WiFi.begin(DEBUG_WIFI_SSID, DEBUG_WIFI_PASSWORD);

  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
  }
#endif
  browser.Start();
}

void loop()
{
  browser.Update();
}
