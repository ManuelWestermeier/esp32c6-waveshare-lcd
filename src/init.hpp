#pragma once

#include <LittleFS.h>

#include "metadata.hpp"
#include "colors.hpp"
#include "rgb-led.hpp"

// void initStorage() {
//   spiSD.begin(SD_SCK, SD_MISO, SD_MOSI, SD_CS);
//   if (!SD.begin(SD_CS, spiSD)) {
//     Serial.println("💥 SD-Karte konnte nicht initialisiert werden!");
//     tft.fillScreen(ST77XX_BLACK);
//     tft.setCursor(0, 0);
//     tft.setTextColor(ST77XX_RED);
//     tft.println("No SD card!");
//     return;
//   }
//   Serial.println("✅ SD-Karte erkannt.");
// }

void mountFS() {
  while (!LittleFS.begin(true)) {
    Serial.println("Failed to mount LittleFS");
    tft.fillScreen(UI_BG);
    tft.setTextColor(UI_Text);
    tft.setCursor(20, 20);
    tft.println("Failed to mount FS");
    delay(2000);
  }
  if (!LittleFS.exists("/users"))
    LittleFS.mkdir("/users");
  if (!LittleFS.exists("/wifi"))
    LittleFS.mkdir("/wifi");
}

void init() {
  Serial.begin(115200);
  Serial.println("Hello");

  pinMode(BUTTON, INPUT_PULLDOWN);
  pixels.begin();
  // SPI init (optional—Adafruit does it for you)
  SPI.begin(TFT_SCLK, /*MISO*/ -1, TFT_MOSI);

  // Initialize display, width=172, height=320
  tft.init(172, 320);
  tft.setRotation(2);  // vertical
  tft.fillScreen(UI_BG);

  // backlight on
  ledcAttach(TFT_BL, 1000, 10);
  ledcWrite(TFT_BL, 255);

  // text style
  tft.setTextColor(UI_Text);
  tft.setTextSize(2);  // 2× scale

  mountFS();
  // initStorage();
}