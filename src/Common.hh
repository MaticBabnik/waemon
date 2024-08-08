#pragma once
#include <cstdint>
#include <string>

enum class DisplayMode { Center, Zoom, Stretch, Tile, Contain };

class Color {
  public:
    uint8_t r = 0, g = 0, b = 0;
    Color() = default;
    Color(std::string colorString);

    std::string toString();
};

DisplayMode parseDisplayMode(std::string modeString);
std::string displayModeString(DisplayMode dm);
