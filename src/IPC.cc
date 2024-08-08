#include "IPC.hh"
#include "Config.hh"
#include "util/log.hh"
#include <unistd.h>

void sendString(int fd, const std::string &str) {
    auto length  = str.length();
    auto written = write(fd, str.c_str(), length);

    if (written != length) {
        logger::warn(
            "Could not send full payload to fd {} (sent {} of {} bytes)",
            fd,
            written,
            length
        );
    }
}

void replyJson(int fd, json body) { sendString(fd, body.dump()); }

void replyError(int fd, std::string error) {
    replyJson(fd, json{{"status", "error"}, {"error", error}});
}

void handleStatus(WallpaperManager &wm, int fd, json cmd) {
    auto status      = wm.serializeStatus();
    status["status"] = "ok";

    replyJson(fd, status);
}

void handleUpdate(WallpaperManager &wm, int fd, json cmd) {
    if (cmd.size() != 3 || !cmd[1].is_string() || !cmd[2].is_object()) {
        return replyError(fd, "Expected [\"update\", string, object]");
    }

    auto groupName = cmd[1].get<std::string>();

    if (!wm.groups.contains(groupName)) {
        return replyError(fd, "Group doesn't exist.");
    }

    std::optional<Color>            color{};
    std::optional<DisplayMode>      dm{};
    std::shared_ptr<WallpaperImage> wallpaper{nullptr};

    if (cmd[2].contains("wallpaper") && cmd[2]["wallpaper"].is_string()) {
        auto wp = WallpaperCache::get(cmd[2]["wallpaper"].get<std::string>());

        if (wp) wallpaper = wp;
        else return replyError(fd, "Couldn't load image");
    }

    if (cmd[2].contains("backgroundColor")
        && cmd[2]["backgroundColor"].is_string()) {
        Color c{(cmd[2]["backgroundColor"].get<std::string>())};
        color = c;
    }

    if (cmd[2].contains("mode") && cmd[2]["mode"].is_string()) {
        auto m = parseDisplayMode(cmd[2]["mode"].get<std::string>());
        dm     = m;
    }

    wm.groups[groupName]->set(wallpaper, color, dm);

    replyJson(fd, json{{"status", "ok"}});
}

void handleCommand(WallpaperManager &wm, int fd, std::string message) {
    json cmd;

    try {
        cmd = json::parse(message);
    } catch (nlohmann::detail::parse_error &pe) {
        return replyError(fd, "Invalid JSON");
    }

    if (!cmd.is_array()) {
        return replyError(fd, "Expected array");
    }

    if (cmd.size() < 1 || !cmd[0].is_string()) {
        return replyError(fd, "First argument must be a string (cmd name)");
    }

    auto cmdName = cmd[0].get<std::string>();

    if (cmdName == "status") return handleStatus(wm, fd, cmd);
    else if (cmdName == "update") return handleUpdate(wm, fd, cmd);
    else return replyError(fd, "Unknown command");
}