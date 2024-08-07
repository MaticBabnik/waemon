#include "Group.hh"
#include <algorithm>
#include <iostream>
#include <ranges>
#include <utility>

BasicSingleMonitorGroup::BasicSingleMonitorGroup(
    const std::shared_ptr<WaylandOutput> &output
) {
    outputs = std::vector<GroupOutput>();
    bounds  = output->getBounds();

    wallpaper = {};

    outputs.push_back({
        .localBounds      = Rect(0, 0, bounds.w, bounds.h),
        .output           = output,
        .wallpaperSurface = std::make_unique<LayerSurface>(output),
    });
}

const std::string &BasicSingleMonitorGroup::getName() const {
    return outputs[0].output->getName();
}

bool BasicSingleMonitorGroup::matchOutput(std::shared_ptr<WaylandOutput> output
) {
    return false;
}

bool BasicSingleMonitorGroup::removeByWlName(uint32_t wl_name) {
    if (outputs[0].output->getWlName() == wl_name) {
        this->outputs.clear();
        return true;
    }
    return false;
}
