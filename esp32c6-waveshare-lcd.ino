#include "src/colors.hpp"
#include "src/metadata.hpp"
#include "src/input.hpp"
#include "src/init.hpp"
#include "src/init-screen.hpp"

#define USE_INIT_SCREEN false

void setup() {
  init();
  Input::start();
#if USE_INIT_SCREEN
  initScreen();
#endif
}

#include "src/select.hpp"
#include "src/ask.hpp"

void loop() {
  std::vector<String> options;
  options.push_back("Hello");
  options.push_back("Nothing");
  options.push_back("NiggaThing");
  for (int i = 0; i < 50; i++) {
    options.push_back("Option" + String(i));
  }
  ok(" What is your\n Wifi password" + String((select(options))));
}
