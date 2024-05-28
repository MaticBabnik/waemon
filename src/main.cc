#include "WallpaperGroup.hh"
#include "WallpaperManager.hh"
#include <memory>
#include <set>

[[noreturn]]
int main() {
    WallpaperManager w(nullptr);

    w.init();

    w.debugAddGroup(std::make_unique<SpanGroup>(
        "Span1",
        std::set<std::string>{"DP-2", "DP-3"}
    ));

    while (true) {
        w.dispatch();
    }
}