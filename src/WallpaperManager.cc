#include "WallpaperManager.hh"
#include <iostream>
#include <ranges>

WallpaperManager::WallpaperManager(const char *display)
    : WaylandManager(display), groups() {}

void WallpaperManager::debugAddGroup(std::unique_ptr<BaseWallpaperGroup> wg) {
    groups.emplace(wg->getName(), std::move(wg)); // FIXME: name collisions!
}

void WallpaperManager::inputReady(uint32_t wlName) {
    auto output = WaylandManager::outputs[wlName];

    for (auto &group : groups | std::views::values) {
        if (group->matchOutput(output)) {
            std::print(
                std::cerr,
                "Grouped output '{}'({}) into '{}'\n",
                output->getName(),
                output->getWlName(),
                group->getName()
            );
            return;
        }
    }

    std::print(
        std::cerr,
        "Creating group for output '{}'\n",
        output->getName()
    );

    groups.emplace(
        output->getName(),
        std::move(std::make_unique<BasicSingleMonitorGroup>(output))
    );
}

void WallpaperManager::inputLost(uint32_t wlName) {
    std::print(std::cerr, "Lost output {}\n", wlName);

    for (auto it = groups.begin(); it != groups.end(); ++it) {
        if (it->second->removeByWlName(wlName)) {
            groups.erase(it);
            return;
        }
    }
}