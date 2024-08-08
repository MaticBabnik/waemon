#include "Config.hh"
#include "WallpaperImage.hh"
#include "util/log.hh"
#include <filesystem>
#include <locale>
#include <memory>
#include <nlohmann/json.hpp>
#include <set>
#include <unistd.h>

namespace fs = std::filesystem;

const std::string CONF_NAME = "paper.jsonc";

std::optional<std::string> getFirstConfigPath() {
    if (fs::is_regular_file(CONF_NAME)) {
        return {CONF_NAME};
    }

    auto envConfHome = getenv("XDG_CONFIG_HOME");
    if (envConfHome) {
        fs::path configHome(envConfHome);
        configHome /= CONF_NAME;

        if (fs::is_regular_file(configHome)) {
            return {configHome};
        }
    }

    auto envHome = getenv("HOME");
    if (envHome) {
        fs::path home(envHome);
        home /= ".config";
        home /= CONF_NAME;

        if (fs::is_regular_file(home)) {
            return {home};
        }
    }

    fs::path etc("/etc");
    etc /= CONF_NAME;

    if (fs::is_regular_file(etc)) {
        return {etc};
    }

    return {};
}

Color parseColor(std::string colorString) {
    if ((colorString.length() != 4 && colorString.length() != 7)
        || colorString[0] != '#') {
        logger::warn("Invalid color string: '{}'", colorString);
        return {0, 0, 0};
    }

    auto n = strtoul(colorString.c_str() + 1, nullptr, 16);
    if (colorString.length() == 4) {
        // sepeate nibbles into bytes, then duplicate low nibbles into high
        n = (n & 0xf00) << 8 | (n & 0xf0) << 4 | (n & 0xf);
        n *= 0x11;
    }

    return {(uint8_t)(n >> 16), (uint8_t)(n >> 8), (uint8_t)n};
}

/*
    Parses a DisplayMode (one of: center, zoom, stretch, tile, contain)
*/
DisplayMode parseMode(std::string modeString) {
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

using json = nlohmann::json;

void addGroupsFromConfig(const std::string &path, WallpaperManager &wm) {
    std::ifstream f(path);

    json config = json::parse(f);

    if (!config.contains("groups") || !config["groups"].is_object()) {
        logger::warn("No 'groups' object in config");
        return;
    }

    for (auto &[groupName, group] : config["groups"].items()) {
        std::set<std::string> outputs;

        if (!group.contains("outputs") || !group["outputs"].is_array()) {
            logger::warn("Group {} contains no outputs", groupName);
            continue;
        }

        for (const auto &output : group["outputs"]) {
            if (!output.is_string()) {
                logger::warn(
                    "Group {}.outputs contains non-string values",
                    groupName
                );
                continue;
            }

            outputs.emplace(output.get<std::string>());
        }

        if (outputs.size() == 0) {
            logger::warn("Group {} contains no outputs", groupName);
            continue;
        }

        auto sg = std::make_unique<SpanGroup>(groupName, outputs);

        if (group.contains("wallpaper") && group["wallpaper"].is_string()) {
            auto wp =
                WallpaperCache::get(group["wallpaper"].get<std::string>());

            if (wp.has_value()) sg->setWallpaper(*wp);
        }

        if (group.contains("backgroundColor")
            && group["backgroundColor"].is_string()) {
            auto c = parseColor(group["backgroundColor"].get<std::string>());

            sg->setFillColor(c);
        }

        if (group.contains("mode") && group["mode"].is_string()) {
            auto m = parseMode(group["mode"].get<std::string>());
            sg->setDisplayMode(m);
        }

        wm.addGroup(std::move(sg));
    }
}