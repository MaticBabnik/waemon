#pragma once
#include "Group.hh"
#include "WallpaperManager.hh"
#include "util/color.hh"
#include "util/displayMode.hh"
#include <optional>
#include <string>

/*
    Checks for valid configs in:
        - `./paper.jsonc`
        - `${XDG_CONFIG_HOME}/paper.jsonc`
        - `${HOME}/.config/paper.jsonc`
        - `/etc/paper.jsonc`
*/
std::optional<std::string> getFirstConfigPath();

/*
    Parses colors formated like #fff or #123456
*/
Color parseColor(std::string colorString);

/*
    Parses a DisplayMode (one of: center, zoom, stretch, tile, contain)
*/
DisplayMode parseMode(std::string modeString);

void addGroupsFromConfig(const std::string& path, WallpaperManager& wm);