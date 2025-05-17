#include <Arduino.h>

#include "src/colors.hpp"
#include "src/metadata.hpp"
#include "src/input.hpp"
#include "src/init.hpp"
#include "src/init-screen.hpp"
#include "src/start.hpp"

#define USE_INIT_SCREEN false

void setup()
{
  init();
  Input::start();
#if USE_INIT_SCREEN
  initScreen();
#endif
  start();
}

void loop()
{
}
