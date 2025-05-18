#include <Arduino.h>

#include "src/colors.hpp"
#include "src/metadata.hpp"
#include "src/input.hpp"
#include "src/init.hpp"
#include "src/init-screen.hpp"
#include "src/start.hpp"

// #define USE_INIT_SCREEN
#define USE_AUTH

Credentials credentials;

void setup() {
  init();
  Input::start();
#ifdef USE_INIT_SCREEN
  initScreen();
#endif
#ifdef USE_AUTH
  credentials = start();
#else
  credentials = { "h", "h", "aaa9402664f1a41f40ebbc52c9993eb66aeb366602958fdfaa283b71e64db123", "aaa9402664f1a41f40ebbc52c9993eb66aeb366602958fdfaa283b71e64db123" };
  WiFi.begin("LocalHost", "hhhhhhhy");

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
  }
#endif
}

#include "inc/qrcode.h"

void drawQRCode(const char *text, int x, int y, int scale) {
  QRCode qrcode;
  uint8_t qrcodeData[qrcode_getBufferSize(3)];  // version 3 = 29x29 matrix

  qrcode_initText(&qrcode, qrcodeData, 3, ECC_LOW, text);

  int size = qrcode.size;

  tft.fillRect(x, y, size * scale, size * scale, UI_BG);  // clear space

  for (int row = 0; row < size; row++) {
    for (int col = 0; col < size; col++) {
      if (qrcode_getModule(&qrcode, col, row)) {
        tft.fillRect(x + col * scale, y + row * scale, scale, scale, UI_BG);
      }
    }
  }
}

void loop() {
  String ip = "http://" + WiFi.localIP().toString();
  Serial.println("IP address: " + ip);

  // Draw the QR code with the IP address
  drawQRCode(ip.c_str(), 20, 20, 4);  // x, y, scale
  tft.setTextColor(UI_BG);
  tft.setTextSize(2);
  tft.setCursor(20, 150);
  tft.println(ip);
  delay(20000);
}
