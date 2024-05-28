#pragma once

#include <stdint.h>

class Rect {
  public:
    Rect(uint32_t x, uint32_t y, uint32_t w, uint32_t h);

    Rect boundingBoxWith(const Rect &that) const;

    uint32_t getBottom() const;
    uint32_t getRight() const;

    uint32_t x, y, w, h;
};