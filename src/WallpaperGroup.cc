#include "WallpaperGroup.hh"
#include <algorithm>
#include <iostream>
#include <ranges>
#include <utility>

BasicSingleMonitorGroup::BasicSingleMonitorGroup(
    std::shared_ptr<WaylandOutput> output
) {
    outputs = std::vector<GroupOutput>();
    bounds  = output->getBounds();

    outputs.push_back({
        .localBounds      = Rect(0, 0, bounds.w, bounds.h),
        .output           = output,
        .wallpaperSurface = std::make_unique<LayerSurface>(output),
    });

    outputs[0].wallpaperSurface->paint();
}

const std::string &BasicSingleMonitorGroup::getName() const {
    return outputs[0].output->getName();
}

bool BasicSingleMonitorGroup::matchOutput(std::shared_ptr<WaylandOutput> output
) {
    return false; // Single Monitor group should remain... single... monitor?
}

bool BasicSingleMonitorGroup::removeByWlName(uint32_t wl_name) {
    if (outputs[0].output->getWlName() == wl_name) {
        this->outputs.clear();
        return true;
    }
    return false;
}

SpanGroup::SpanGroup(std::string _name, std::set<std::string> _outputNames)
    : name(std::move(_name)), outputNames(std::move(_outputNames)) {}

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
        return true;
    }

    return false;
}

void SpanGroup::recomputeBounds() {
    auto boundingBox =
        !outputs.empty() ? Rect(0, 0, 0, 0) : outputs[0].output->getBounds();

    for (const auto &o : outputs | std::views::drop(1)) {
        boundingBox = boundingBox.boundingBoxWith(o.output->getBounds());
    }
    this->bounds = boundingBox;

    std::print(
        std::cout,
        "RecomputeBounds:  ({},{}) ({}x{})\n",
        boundingBox.x,
        boundingBox.y,
        boundingBox.w,
        boundingBox.h
    );

    // make localBounds local
    for (auto &o : outputs) {
        auto b = o.output->getBounds();

        o.localBounds.x = b.x - boundingBox.x;
        o.localBounds.y = b.y - boundingBox.y;

        std::print(
            std::cout,
            "\t{} ({},{}) ({}x{})\n",
            o.output->getName(),
            o.localBounds.x,
            o.localBounds.y,
            b.w,
            b.h
        );
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

    return false; // don't deletee empty SpanGroups (this makes sense, trust)
}
