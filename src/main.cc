#include "Config.hh"
#include "Group.hh"
#include "Socket.hh"
#include "WallpaperImage.hh"
#include "WallpaperManager.hh"
#include "util/log.hh"
#include "util/panic.hh"

std::string getSocketPath() {
    auto rtdir = getenv("XDG_RUNTIME_DIR");
    if (!rtdir) panic("No XDG_RUNTIME_DIR");

    return std::string(rtdir) + "/paper.sock";
}

[[noreturn]]
int main(int argc, char *argv[]) {

    auto configPath = getFirstConfigPath();
    if (!configPath.has_value()) {
        panic("No configuration files found");
    }

    WallpaperManager w(nullptr);

    Socket sock{getSocketPath()};

    logger::info("Using config file: {}", *configPath);
    addGroupsFromConfig(*configPath, w);

    while (true) {
        w.dispatch();

        sock.dispatch([](std::string &msg, int fd) {
            logger::info("Read({}): {}", fd, msg);

            std::string response{"Hello"};
            write(fd, response.c_str(), response.length() + 1);
        });
    }
}
