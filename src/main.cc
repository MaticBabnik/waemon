#include "Config.hh"
#include "Group.hh"
#include "WallpaperImage.hh"
#include "WallpaperManager.hh"
#include "util/log.hh"
#include "util/panic.hh"
#include <memory>
#include <set>

[[noreturn]]
int main(int argc, char *argv[]) {
    auto configPath = getFirstConfigPath();
    if (!configPath.has_value()) {
        panic("No configuration files found");
    }

    WallpaperManager w(nullptr);

    logger::info("Using config file: {}", *configPath);
    addGroupsFromConfig(*configPath, w);

    while (true) {
        w.dispatch();
    }
}
