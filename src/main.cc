#include "WallpaperGroup.hh"
#include "WallpaperImage.hh"
#include "WallpaperManager.hh"
#include <memory>
#include <set>

[[noreturn]]
int main(int argc, char *argv[]) {
    std::shared_ptr<WallpaperImage> img;

    if (argc > 1) {
        img = std::make_shared<WallpaperImage>(argv[1]);
    } else {
        img = std::make_shared<WallpaperImage>("../testW.png");
    }

    WallpaperManager w(nullptr);

    w.debugAddGroup(std::make_unique<SpanGroup>(
        "Span1",
        std::set<std::string>{"DP-2", "DP-3"}
    ));
    w.groups["Span1"]->setWallpaper(img);
    w.groups["Span1"]->setDisplayMode(DisplayMode::Zoom);
    w.groups["Span1"]->setFillColor(255,100,200);

    while (true) {
        w.dispatch();
    }
}