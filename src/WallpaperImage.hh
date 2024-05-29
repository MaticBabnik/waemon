#pragma once
#include "util/math2d.hh"
#include <OpenImageIO/imageio.h>
#include <cairo/cairo.h>
#include <string>

class WallpaperImage {
  public:
    cairo_surface_t *surface;

    explicit WallpaperImage(const std::string &path);
    ~WallpaperImage();

    Vec2<int> size() const;

  protected:
    bool fixColorFormat(std::string &colorFormat);

    uint32_t  width, height;
    uint32_t *pixeldata;
};