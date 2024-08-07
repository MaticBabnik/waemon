#include "Group.hh"
#include <algorithm>
#include <iostream>
#include <ranges>
#include <utility>

SpanGroup::SpanGroup(std::string _name, std::set<std::string> _outputNames)
    : name(std::move(_name)), outputNames(std::move(_outputNames)) {
    wallpaper = {};
}

const std::string &SpanGroup::getName() const { return name; }

bool SpanGroup::matchOutput(std::shared_ptr<WaylandOutput> output) {
    if (outputNames.contains(output->getName())) {
        auto bounds = output->getBounds();

        outputs.push_back({
            .localBounds      = Rect(0, 0, bounds.w, bounds.h),
            .output           = output,
            .wallpaperSurface = std::make_unique<LayerSurface>(output),
        });

        recomputeBounds();
        applyWallpaper();
        return true;
    }

    return false;
}

void SpanGroup::recomputeBounds() {
    auto boundingBox =
        outputs.empty() ? Rect(0, 0, 0, 0) : outputs[0].output->getBounds();

    for (const auto &o : outputs | std::views::drop(1)) {
        boundingBox = boundingBox.boundingBoxWith(o.output->getBounds());
    }
    this->bounds = boundingBox;

    // make localBounds local
    for (auto &o : outputs) {
        auto b = o.output->getBounds();

        o.localBounds.x = b.x - boundingBox.x;
        o.localBounds.y = b.y - boundingBox.y;
    }
}

bool SpanGroup::removeByWlName(uint32_t wl_name) {

    outputs.erase(
        std::remove_if(
            outputs.begin(),
            outputs.end(),
            [wl_name](const auto &x) {
                return x.output->getWlName() == wl_name;
            }
        ),
        outputs.end()
    );

    recomputeBounds();
    if (!outputs.empty()) {
        applyWallpaper();
    }

    return false;
}
