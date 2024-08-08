#pragma once

#include "WallpaperManager.hh"
#include <nlohmann/json.hpp>
#include <string>

using json = nlohmann::json;

void replyJson(int fd, json body);
void replyError(int fd, std::string error);

void handleCommand(WallpaperManager &wm, int fd, std::string message);