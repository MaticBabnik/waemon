#pragma once

#include <cstdint>

class Rect {
  public:
    Rect(int32_t x, int32_t y, int32_t w, int32_t h);

    Rect boundingBoxWith(const Rect &that) const;

    int32_t getBottom() const;
    int32_t getRight() const;

    int32_t x, y, w, h;
};