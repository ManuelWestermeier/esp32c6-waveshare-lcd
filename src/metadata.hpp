#pragma once

#include <Arduino.h>
#include <vector>

using namespace std;

#define TFT_MOSI 6
#define TFT_SCLK 7
#define TFT_CS 14
#define TFT_DC 15
#define TFT_RST 21 // or tie to 3.3 V if you don’t want to reset
#define TFT_BL 22

#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ST7789.h>

// Instantiate with: (CS, DC, RST)
Adafruit_ST7789 tft = Adafruit_ST7789(TFT_CS, TFT_DC, TFT_RST);

#define SD_CS    4
#define SD_MOSI  6
#define SD_MISO  5
#define SD_SCK   7

#include <SD.h>
SPIClass spiSD(FSPI);

#include <Adafruit_NeoPixel.h>
#define RGB_LED_PIN 8 // GPIO8 on ESP32
#define NUMPIXELS 1   // Only 1 RGB LED

Adafruit_NeoPixel pixels(NUMPIXELS, RGB_LED_PIN, NEO_GRB + NEO_KHZ800);