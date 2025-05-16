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
  ok(" What is your\n Wifi password" + String(options.at(select(options))));
}
