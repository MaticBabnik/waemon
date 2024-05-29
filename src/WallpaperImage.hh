#pragma once
#include <OpenImageIO/imageio.h>
#include <cairo/cairo.h>
#include <string>

class WallpaperImage {
  public:
    cairo_surface_t *surface;

    explicit WallpaperImage(const std::string &path);
    ~WallpaperImage();

  protected:
    bool fixColorFormat(std::string &colorFormat);

    uint32_t  width, height;
    uint32_t *pixeldata;
};