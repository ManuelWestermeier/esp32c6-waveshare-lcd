#include "colors.hpp"
#include "metadata.hpp"
#include "input.hpp"
#include "init.hpp"
#include "init-screen.hpp"

#define USE_INIT_SCREEN true

void setup() {
  init();
  Input::start();
#if USE_INIT_SCREEN
  initScreen();
#endif
}

void loop() {
}
