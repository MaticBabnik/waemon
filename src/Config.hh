#pragma once
#include "Common.hh"
#include "Group.hh"
#include "WallpaperManager.hh"
#include <optional>
#include <string>

#ifndef PROGRAM_NAME
#define PROGRAM_NAME "waemon"
#endif

/*
    Checks for valid configs in:
        - `./waemon.jsonc`
        - `${XDG_CONFIG_HOME}/waemon.jsonc`
        - `${HOME}/.config/waemon.jsonc`
        - `/etc/waemon.jsonc`
*/
std::optional<std::string> getFirstConfigPath();

std::string getSocketPath();
std::string getLockPath();

void addGroupsFromConfig(const std::string &path, WallpaperManager &wm);