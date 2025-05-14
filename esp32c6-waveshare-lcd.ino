#include "metadata.hpp"
#include "input.hpp"
#include "init.hpp"

void setup() {
  init();
  Input::start();
  
  tft.println();
  tft.println(" Start");
  tft.setTextColor(0x051f);
}

void loop() {
  auto lastEvent = Input::getLastEvent();
  if (lastEvent != Input::None) {
    if(lastEvent == Input::Click) {
      tft.println(" Click");
    }
    if(lastEvent == Input::LongPress) {
      tft.println(" LongPress");
    }
    if(lastEvent == Input::DoubleClick) {
      tft.println(" DoubleClick");
    }
    if(lastEvent == Input::TripleClick) {
      tft.println(" TripleClick");
    }
    delay(300);
  }
}
