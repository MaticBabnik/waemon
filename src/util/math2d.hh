#pragma once
#include <algorithm>

template <typename T> class Vec2 {
  public:
    T x, y;

    Vec2(T x, T y) {
        this->x = x;
        this->y = y;
    }

    template <typename U>
    explicit Vec2(const Vec2<U> &other)
        : x(static_cast<T>(other.x)), y(static_cast<T>(other.y)) {}

    Vec2<T> operator+(const Vec2<T> &other) const {
        return {x + other.x, y + other.y};
    }

    Vec2<T> operator-(const Vec2<T> &other) const {
        return {x - other.x, y - other.y};
    }

    Vec2<T> operator-() const { return {-x, -y}; }

    template <typename U> Vec2<T> operator*(U scalar) const {
        return {x * scalar, y * scalar};
    }

    template <typename U> Vec2<T> operator/(U scalar) const {
        return {x / scalar, y / scalar};
    }
};

template <typename T> class Rect {
  public:
    T x, y, w, h;

    Rect(T x, T y, T w, T h) {
        this->x = x;
        this->y = y;
        this->w = w;
        this->h = h;
    }

    T getBottom() const { return y + h; }

    T getRight() const { return x + w; }

    Vec2<T> origin() const { return {x, y}; }

    Vec2<T> size() const { return {w, h}; }

    Rect<T> boundingBoxWith(const Rect<T> &that) const {
        auto top    = std::min(y, that.y);
        auto left   = std::min(x, that.x);
        auto right  = std::max(getRight(), that.getRight());
        auto bottom = std::max(getBottom(), that.getBottom());

        return {left, top, right - left, bottom - top};
    }
};
