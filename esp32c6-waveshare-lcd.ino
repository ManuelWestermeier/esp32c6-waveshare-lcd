#include "metadata.hpp"

#define BUTTON 1

void setup() {
  pinMode(BUTTON, INPUT_PULLDOWN);
  // SPI init (optional—Adafruit does it for you)
  SPI.begin(TFT_SCLK, /*MISO*/ -1, TFT_MOSI);

  // Initialize display, width=172, height=320
  tft.init(172, 320);
  tft.setRotation(2);  // match your orientation
  tft.fillScreen(0xdfbb);

  // backlight on
  ledcAttach(TFT_BL, 1000, 10);
  ledcWrite(TFT_BL, 255);

  // draw some text
  tft.setTextColor(0x00c1);
  tft.setTextSize(2);  // 2× scale
  tft.println();
  tft.println(" Hello");
  tft.setTextColor(0x02e2);
}

void loop() {
  if (digitalRead(BUTTON) == HIGH) {
    tft.println(" Click");
    delay(300);
  }
}
