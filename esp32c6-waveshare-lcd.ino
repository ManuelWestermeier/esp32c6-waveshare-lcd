#define TFT_MOSI   6
#define TFT_SCLK   7
#define TFT_CS     14
#define TFT_DC     15
#define TFT_RST    21  // or tie to 3.3 V if you don’t want to reset
#define TFT_BL     22
#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ST7789.h>

// Instantiate with: (CS, DC, RST)
Adafruit_ST7789 tft = Adafruit_ST7789(TFT_CS, TFT_DC, TFT_RST);

void setup() {
  // SPI init (optional—Adafruit does it for you)
  SPI.begin(TFT_SCLK, /*MISO*/ -1, TFT_MOSI);

  // Initialize display, width=172, height=320
  tft.init(172, 320);
  tft.setRotation(1);                // match your orientation
  tft.fillScreen(ST77XX_BLACK);

  // backlight on
  ledcAttach(TFT_BL, 1000, 10);
  ledcWrite(TFT_BL, 255);

  // draw some text
  tft.setTextColor(ST77XX_WHITE);
  tft.setTextSize(2);                // 2× scale
  tft.setCursor(10, 10);
  tft.println("Hello, Niggas!");
  tft.setCursor(10, 40);
  tft.setTextColor(ST77XX_RED);
  tft.println("ST7789 + GFX");
}

void loop() {
  // your code…
}
