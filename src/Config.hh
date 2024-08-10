#pragma once
#include "Common.hh"
#include "Group.hh"
#include "WallpaperManager.hh"
#include <optional>
#include <string>

#ifndef PROGRAM_NAME
#define PROGRAM_NAME "paper"
#endif

/*
    Checks for valid configs in:
        - `./paper.jsonc`
        - `${XDG_CONFIG_HOME}/paper.jsonc`
        - `${HOME}/.config/paper.jsonc`
        - `/etc/paper.jsonc`
*/
std::optional<std::string> getFirstConfigPath();

std::string getSocketPath();
std::string getLockPath();

void addGroupsFromConfig(const std::string &path, WallpaperManager &wm);