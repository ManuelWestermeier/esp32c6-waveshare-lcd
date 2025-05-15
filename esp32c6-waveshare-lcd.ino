#include "colors.hpp"
#include "metadata.hpp"
#include "input.hpp"
#include "init.hpp"
#include "init-screen.hpp"

void setup() {
  init();
  Input::start();
  initScreen();
}

void loop() {
}
