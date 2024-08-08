#pragma once

#include "Group.hh"
#include "wayland/WaylandManager.hh"
#include <map>
#include <memory>
#include <nlohmann/json.hpp>
#include <string>

using json = nlohmann::json;

class WallpaperManager : public WaylandManager {
  public:
    explicit WallpaperManager(const char *display);
    void addGroup(std::unique_ptr<BaseWallpaperGroup> wg);
    std::map<std::string, std::unique_ptr<BaseWallpaperGroup>> groups;

    json serializeStatus();

  protected:
    void inputReady(uint32_t wlName) override;
    void inputLost(uint32_t wlName) override;
};