#pragma once

#include "WallpaperGroup.hh"
#include "wayland/WaylandManager.hh"
#include <map>
#include <memory>
#include <string>

class WallpaperManager : public WaylandManager {
  public:
    WallpaperManager(const char *display);
    void debugAddGroup(std::unique_ptr<BaseWallpaperGroup> wg);
    std::map<std::string, std::unique_ptr<BaseWallpaperGroup>> groups;

  protected:
    void inputReady(uint32_t wlName) override;
    void inputLost(uint32_t wlName) override;
};