#include "rect.hh"
#include <algorithm>

Rect::Rect(uint32_t x, uint32_t y, uint32_t w, uint32_t h) {
    this->x = x;
    this->y = y;
    this->w = w;
    this->h = h;
}

uint32_t Rect::getBottom() const { return y + h; }

uint32_t Rect::getRight() const { return x + w; }

Rect Rect::boundingBoxWith(const Rect &that) const {
    auto top    = std::min(y, that.y);
    auto left   = std::min(x, that.x);
    auto right  = std::max(getRight(), that.getRight());
    auto bottom = std::max(getBottom(), that.getBottom());

    return Rect(left, top, right - left, bottom - top);
}