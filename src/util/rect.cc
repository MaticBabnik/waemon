#include "rect.hh"
#include <algorithm>

Rect::Rect(int32_t x, int32_t y, int32_t w, int32_t h) {
    this->x = x;
    this->y = y;
    this->w = w;
    this->h = h;
}

int32_t Rect::getBottom() const { return y + h; }

int32_t Rect::getRight() const { return x + w; }

Rect Rect::boundingBoxWith(const Rect &that) const {
    auto top    = std::min(y, that.y);
    auto left   = std::min(x, that.x);
    auto right  = std::max(getRight(), that.getRight());
    auto bottom = std::max(getBottom(), that.getBottom());

    return Rect(left, top, right - left, bottom - top);
}