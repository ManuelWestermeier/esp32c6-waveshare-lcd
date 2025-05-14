#pragma once

#include "./metadata.hpp"
#include <math.h>

// helper: draw a vertical gradient from topColor to bottomColor
void drawVerticalGradient(uint16_t topColor, uint16_t bottomColor) {
  uint16_t h = tft.height();
  uint8_t rt = (topColor >> 11) & 0x1F, gt = (topColor >> 5) & 0x3F, bt = topColor & 0x1F;
  uint8_t rb = (bottomColor >> 11) & 0x1F, gb = (bottomColor >> 5) & 0x3F, bb = bottomColor & 0x1F;
  for (uint16_t y = 0; y < h; y++) {
    float f = float(y) / (h - 1);
    uint8_t r = rt + (rb - rt) * f;
    uint8_t g = gt + (gb - gt) * f;
    uint8_t b = bt + (bb - bt) * f;
    tft.drawFastHLine(0, y, tft.width(), tft.color565(r << 3, g << 2, b << 3));
  }
}

// project a 3D point (x,z) onto screen with simple perspective
void projPoint(float x, float y, float z, int16_t &sx, int16_t &sy) {
  const float dist = 3.0;      // camera distance
  const float scale = 100.0;   // overall size
  float inv = scale / (z + dist);
  sx = int16_t(x * inv + tft.width() * 0.5);
  sy = int16_t(y * inv + tft.height() * 0.45);
}

void initScreen() {
  tft.setRotation(1);
  
  const uint16_t gradTop = tft.color565(20,  0,  60);  // deep violet
  const uint16_t gradBot = tft.color565( 0, 120, 100); // teal
  
  float angle = 0;
  
  // animate for ~6 seconds
  unsigned long start = millis();
  while (millis() - start < 6000) {
    // 1) background gradient
    drawVerticalGradient(gradTop, gradBot);

    // 2) rotating 3D ring of particles
    const int N = 80;
    for (int i = 0; i < N; i++) {
      float phi = (2 * M_PI * i) / N;
      // basic torus cross‑section circle
      float cx = (1.0 + 0.3 * cos(phi)) * cos(angle);
      float cy = 0.3 * sin(phi);
      float cz = (1.0 + 0.3 * cos(phi)) * sin(angle);
      
      int16_t sx, sy;
      projPoint(cx, cy, cz, sx, sy);
      
      // color gradient based on depth (cz)
      uint8_t bright = uint8_t(150 + (cz * 105));  
      tft.drawFastVLine(sx, sy, 2, tft.color565(bright, bright/2, 255 - bright/2));
    }

    // 3) “MW OS” with 3D drop‑shadow layers
    tft.setTextSize(6);
    for (int layer = 6; layer > 0; layer--) {
      // offset shadow
      tft.setTextColor(tft.color565(0, 0, layer * 20)); 
      tft.setCursor(90 + layer,  80 + layer);
      tft.print("MW");
      tft.setCursor(90 + layer, 150 + layer);
      tft.print("OS");
    }
    // main text in glowing gradient
    tft.setTextColor(tft.color565(255, 200, 10));
    tft.setCursor(90,  80);
    tft.print("MW");
    tft.setCursor(90, 150);
    tft.print("OS");

    delay(30);
    angle += 0.08;
  }

  // final clear to your UI background
  tft.fillScreen(UI_BG);
}
