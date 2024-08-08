#include "WallpaperManager.hh"
#include "util/log.hh"
#include <iostream>
#include <ranges>

WallpaperManager::WallpaperManager(const char *display)
    : WaylandManager(display), groups() {}

void WallpaperManager::addGroup(std::unique_ptr<BaseWallpaperGroup> wg) {
    groups.emplace(wg->getName(), std::move(wg)); // FIXME: name collisions!
}

void WallpaperManager::inputReady(uint32_t wlName) {
    auto output = WaylandManager::outputs[wlName];

    for (auto &group : groups | std::views::values) {
        if (group->matchOutput(output)) {
            logger::info(
                "Grouped output '{}' into '{}'",
                output->getName(),
                group->getName()
            );
            return;
        }
    }

    logger::warn(
        "Creating placeholder group for output '{}'",
        output->getName()
    );

    groups.emplace(
        output->getName(),
        std::move(std::make_unique<BasicSingleMonitorGroup>(output))
    );
}

void WallpaperManager::inputLost(uint32_t wlName) {
    logger::info("Lost output {}", wlName);

    for (auto it = groups.begin(); it != groups.end(); ++it) {
        if (it->second->removeByWlName(wlName)) {
            groups.erase(it);
            return;
        }
    }
}

json WallpaperManager::serializeStatus() {
    json body{{"groups", {}}};

    for (const auto &p : groups) {
        body["groups"][p.first] = p.second->serializeStatus();
    }

    return body;
}