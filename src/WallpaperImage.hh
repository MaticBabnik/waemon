#pragma once
#include "util/math2d.hh"
#include <OpenImageIO/imageio.h>
#include <cairo/cairo.h>
#include <map>
#include <memory>
#include <optional>
#include <string>



class WallpaperImage {
  public:
    explicit WallpaperImage(const std::string &path);
    ~WallpaperImage();

    bool      isValid() const;
    Vec2<int> size() const;

    cairo_surface_t *getSurface();

  protected:
    bool fixColorFormat(std::string &colorFormat);
    bool valid = true;

    uint32_t width, height;

    cairo_surface_t *surface   = nullptr;
    uint32_t        *pixeldata = nullptr;
};

class WallpaperCache {
  public:
    static std::optional<std::shared_ptr<WallpaperImage>>
    get(const std::string &path);

  private:
    static std::map<std::string, std::weak_ptr<WallpaperImage>> cache;
};