#pragma once

#include "WallpaperImage.hh"
#include "util/math2d.hh"
#include "wayland/LayerSurface.hh"
#include "wayland/WaylandOutput.hh"
#include <memory>
#include <optional>
#include <set>
#include <string>
#include <tuple>
#include <vector>

class WaylandOutput;

enum class DisplayMode { Center, Zoom, Stretch, Tile, Contain };

struct GroupOutput {
    Rect<int32_t>                  localBounds;
    std::shared_ptr<WaylandOutput> output;
    std::unique_ptr<LayerSurface>  wallpaperSurface;
};

class BaseWallpaperGroup {
  public:
    virtual bool matchOutput(std::shared_ptr<WaylandOutput> output) = 0;
    virtual bool removeByWlName(uint32_t wl_name)                   = 0;

    virtual const std::string &getName() const = 0;
    void setWallpaper(const std::shared_ptr<WallpaperImage> &image);
    void setFillColor(uint8_t r, uint8_t g, uint8_t b);
    void setDisplayMode(DisplayMode dm);

  protected:
    virtual void applyWallpaper();

    Rect<int32_t>            bounds{0, 0, 0, 0};
    std::vector<GroupOutput> outputs;

    std::tuple<uint8_t, uint8_t, uint8_t>          fill_color;
    std::optional<std::shared_ptr<WallpaperImage>> wallpaper;

    DisplayMode displayMode{DisplayMode::Center};
};

class BasicSingleMonitorGroup : public BaseWallpaperGroup {
  public:
    explicit BasicSingleMonitorGroup(
        const std::shared_ptr<WaylandOutput> &output
    );

    const std::string &getName() const override;
    bool matchOutput(std::shared_ptr<WaylandOutput> output) override;
    bool removeByWlName(uint32_t wl_name) override;
};

class SpanGroup : public BaseWallpaperGroup {
  public:
    SpanGroup(std::string name, std::set<std::string> outputNames);

    const std::string &getName() const override;
    bool matchOutput(std::shared_ptr<WaylandOutput> output) override;
    bool removeByWlName(uint32_t wl_name) override;

  private:
    std::string           name;
    std::set<std::string> outputNames;

    void recomputeBounds();
};