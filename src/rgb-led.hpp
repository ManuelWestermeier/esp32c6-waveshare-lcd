#pragma once

#include "metadata.hpp"
#include "colors.hpp"

void setRGB_LEDColor(Color color)
{
    pixels.setPixelColor(0, pixels.Color(color.red(), color.green(), color.blue()));
    pixels.show();
}
