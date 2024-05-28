#include "WallpaperGroup.hh"
#include "WallpaperImage.hh"
#include "WallpaperManager.hh"
#include <memory>
#include <set>

[[noreturn]]
int main() {
    auto img = std::make_shared<WallpaperImage>("../testW.png");

    WallpaperManager w(nullptr);

    w.debugAddGroup(std::make_unique<SpanGroup>(
        "Span1",
        std::set<std::string>{"DP-2", "DP-3"}
    ));
    w.groups["Span1"]->setWallpaper(img);

    while (true) {
        w.dispatch();
    }
}