#include "src/colors.hpp"
#include "src/metadata.hpp"
#include "src/input.hpp"
#include "src/init.hpp"
#include "src/init-screen.hpp"

#define USE_INIT_SCREEN true

void setup()
{
  init();
  Input::start();
#if USE_INIT_SCREEN
  initScreen();
#endif
}

void loop()
{
}
