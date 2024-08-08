#include "Common.hh"
#include "util/log.hh"
#include <algorithm>
#include <cstdlib>
#include <locale>

Color::Color(std::string colorString) {
    if ((colorString.length() != 4 && colorString.length() != 7)
        || colorString[0] != '#') {
        logger::warn("Invalid color string: '{}'", colorString);
        return;
    }

    auto n = strtoul(colorString.c_str() + 1, nullptr, 16);

    if (colorString.length() == 4) {
        // sepeate nibbles into bytes, then duplicate low nibbles into high
        n = (n & 0xf00) << 8 | (n & 0xf0) << 4 | (n & 0xf);
        n *= 0x11;
    }

    this->r = (uint8_t)(n >> 16);
    this->g = (uint8_t)(n >> 8);
    this->b = (uint8_t)(n);
}

std::string Color::toString() {
    char color[8];
    sprintf(color, "#%02x%02x%02x", r, g, b);

    return {color};
}

/*
    Parses a DisplayMode (one of: center, zoom, stretch, tile, contain)
*/
DisplayMode parseDisplayMode(std::string modeString) {
    std::transform(
        modeString.begin(),
        modeString.end(),
        modeString.begin(),
        [](unsigned char c) { return std::tolower(c); }
    );

    if (modeString == "center") {
        return DisplayMode::Center;
    } else if (modeString == "zoom") {
        return DisplayMode::Zoom;
    } else if (modeString == "stretch") {
        return DisplayMode::Stretch;
    } else if (modeString == "tile") {
        return DisplayMode::Tile;
    } else if (modeString == "contain") {
        return DisplayMode::Contain;
    } else {
        logger::warn("Invalid display mode: '{}'", modeString);
        return DisplayMode::Zoom;
    }
}

std::string displayModeString(DisplayMode dm) {
    switch (dm) {
    case DisplayMode::Center:
        return "center";
    case DisplayMode::Zoom:
        return "zoom";
    case DisplayMode::Stretch:
        return "stretch";
    case DisplayMode::Tile:
        return "tile";
    case DisplayMode::Contain:
        return "contain";
    default:
        return "unknown!?";
    }
}
