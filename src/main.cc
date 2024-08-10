#include "Config.hh"
#include "Group.hh"
#include "IPC.hh"
#include "WallpaperImage.hh"
#include "WallpaperManager.hh"
#include "util/Socket.hh"
#include "util/lock.hh"
#include "util/log.hh"
#include "util/panic.hh"

[[noreturn]]
int main(int argc, char *argv[]) {
    ExclusiveLock lock{getLockPath()};
    if (!lock.locked()) {
        // TODO: Maybe kill the other instance, so that WMs can "reload" us.
        // TODO: Maybe tell the other instance to seppuku over IPC
        panic("Couldn't obtain lock. Is another instance running?");
    }

    WallpaperManager w(nullptr);
    Socket           sock{getSocketPath()};

    auto configPath = getFirstConfigPath();
    if (configPath.has_value()) {
        logger::info("Using config file: {}", *configPath);
        addGroupsFromConfig(*configPath, w);
    } else {
        logger::warn("No configuration files found");
    }

    while (true) {
        w.dispatch();

        sock.dispatch([&w](std::string &msg, int fd) {
            handleCommand(w, fd, msg);
        });
    }
}
