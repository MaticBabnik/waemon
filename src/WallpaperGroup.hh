#pragma once

#include <memory>
#include <set>
#include <string>
#include <vector>

#include "util/rect.hh"
#include "wayland/LayerSurface.hh"
#include "wayland/WaylandOutput.hh"

class WaylandOutput;

struct GroupOutput {
    Rect                           localBounds;
    std::shared_ptr<WaylandOutput> output;
    std::unique_ptr<LayerSurface>  wallpaperSurface;
};

class BaseWallpaperGroup {
  public:
    virtual bool matchOutput(std::shared_ptr<WaylandOutput> output) = 0;
    virtual bool removeByWlName(uint32_t wl_name)                   = 0;
    virtual const std::string &getName() const                      = 0;

  protected:
    Rect                     bounds{0, 0, 0, 0};
    std::vector<GroupOutput> outputs;
};

class BasicSingleMonitorGroup : public BaseWallpaperGroup {
  public:
    explicit BasicSingleMonitorGroup(std::shared_ptr<WaylandOutput> output);

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