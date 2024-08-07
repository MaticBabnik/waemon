#pragma once

#include "WallpaperImage.hh"
#include "util/color.hh"
#include "util/math2d.hh"
#include "util/displayMode.hh"
#include "wayland/LayerSurface.hh"
#include "wayland/WaylandOutput.hh"
#include <memory>
#include <optional>
#include <set>
#include <string>
#include <vector>

class WaylandOutput;

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
    void setFillColor(const Color &c);
    void setDisplayMode(DisplayMode dm);

  protected:
    virtual void applyWallpaper();

    Color                    fill_color;
    Rect<int32_t>            bounds{0, 0, 0, 0};
    std::vector<GroupOutput> outputs;

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