#pragma once

#include <Arduino.h>
#include "../metadata.hpp"

struct Pos
{
    uint16_t x;
    uint16_t y;
};

struct Rect
{
    Pos pos; // Top-left corner
    Pos dem; // Bottom-right corner (inclusive)

    bool collides(Pos other) const
    {
        return other.x >= pos.x && other.x <= dem.x && other.y >= pos.y && other.y <= dem.y;
    }

    bool collides(const Rect &other) const
    {
        return !(other.dem.x < pos.x || other.pos.x > dem.x || other.dem.y < pos.y || other.pos.y > dem.y);
    }
};

struct UI
{
    Rect pos;
    bool isFocused = false;

    void Render();
    void Focus();
    void FocusEnd();
    void Click();
};
