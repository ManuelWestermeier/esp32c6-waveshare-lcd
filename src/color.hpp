#pragma once

#include <Arduino.h>

// 16-bit RGB 5-6-5 color format
struct Color {
  uint16_t data;

  // Default constructor: black
  constexpr Color(uint16_t d = 0)
    : data(d) {}

  // Construct from individual R, G, B components (5-bit R [0-31], 6-bit G [0-63], 5-bit B [0-31])
  constexpr Color(uint8_t r, uint8_t g, uint8_t b)
    : data(static_cast<uint16_t>((r & 0x1F) << 11) | static_cast<uint16_t>((g & 0x3F) << 5) | static_cast<uint16_t>(b & 0x1F)) {}

  // Implicit conversion to uint16_t
  constexpr operator uint16_t() const {
    return data;
  }

  // Set new R,G,B components
  void set(uint8_t r, uint8_t g, uint8_t b) {
    data = static_cast<uint16_t>((r & 0x1F) << 11) | static_cast<uint16_t>((g & 0x3F) << 5) | static_cast<uint16_t>(b & 0x1F);
  }

  // Extract red component (0-31)
  uint8_t red() const {
    return (data >> 11) & 0x1F;
  }

  // Extract green component (0-63)
  uint8_t green() const {
    return (data >> 5) & 0x3F;
  }

  // Extract blue component (0-31)
  uint8_t blue() const {
    return data & 0x1F;
  }
};

/**
 * @brief Linearly interpolate between two colors in 16-bit 5-6-5 format.
 * @param from      Starting color
 * @param to        Target color
 * @param t         Transition parameter in range [0..255]
 * @return Color    Interpolated color
 */
Color colorTransition(const Color &from, const Color &to, uint8_t t) {
  uint8_t r = ((uint16_t)from.red() * (255 - t) + (uint16_t)to.red() * t) / 255;
  uint8_t g = ((uint16_t)from.green() * (255 - t) + (uint16_t)to.green() * t) / 255;
  uint8_t b = ((uint16_t)from.blue() * (255 - t) + (uint16_t)to.blue() * t) / 255;

  return Color(r, g, b);
}
