#include "Config.hh"
#include "WallpaperImage.hh"
#include "util/log.hh"
#include "util/panic.hh"
#include <filesystem>
#include <memory>
#include <nlohmann/json.hpp>
#include <set>
#include <unistd.h>

namespace fs = std::filesystem;

const std::string CONF_NAME = PROGRAM_NAME ".jsonc";

std::string getSocketPath() {
    auto rtdir = getenv("XDG_RUNTIME_DIR");
    if (!rtdir) panic("No XDG_RUNTIME_DIR");

    return std::string(rtdir) + "/" PROGRAM_NAME ".sock";
}

std::string getLockPath() {
    auto rtdir = getenv("XDG_RUNTIME_DIR");
    if (!rtdir) panic("No XDG_RUNTIME_DIR");

    return std::string(rtdir) + "/" PROGRAM_NAME ".lock";
}

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

using json = nlohmann::json;

void addGroupsFromConfig(const std::string &path, WallpaperManager &wm) {
    std::ifstream f(path);

    json config;
    try {
        config = json::parse(f);
    } catch (nlohmann::detail::parse_error &pe) {
        panic("Error parsing config file: {}", pe.what());
    }

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

            if (wp) sg->setWallpaper(wp);
        }

        if (group.contains("backgroundColor")
            && group["backgroundColor"].is_string()) {
            Color c = {group["backgroundColor"].get<std::string>()};

            sg->setFillColor(c);
        }

        if (group.contains("mode") && group["mode"].is_string()) {
            auto m = parseDisplayMode(group["mode"].get<std::string>());
            sg->setDisplayMode(m);
        }

        wm.addGroup(std::move(sg));
    }
}