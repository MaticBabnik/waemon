#include "WallpaperGroup.hh"
#include <algorithm>
#include <iostream>
#include <ranges>
#include <utility>

void BaseWallpaperGroup::setWallpaper(std::shared_ptr<WallpaperImage> img) {
    this->wallpaper = img;
}

void BaseWallpaperGroup::applyWallpaper() {
    if (!wallpaper.has_value()) return;

    for (auto &o : outputs) {
        auto wp = this->wallpaper->get();
        auto r  = o.localBounds;

        o.wallpaperSurface->paint([wp, r](cairo_t *cr) {
            // paint background color
            cairo_set_source_rgb(cr, 0.0, 0.0, 0.0);
            cairo_paint(cr);

            // paint the image
            cairo_set_source_surface(cr, wp->surface, -r.x, -r.y);
            cairo_paint(cr);
        });
    }
}

BasicSingleMonitorGroup::BasicSingleMonitorGroup(
    std::shared_ptr<WaylandOutput> output
) {
    outputs = std::vector<GroupOutput>();
    bounds  = output->getBounds();

    wallpaper = {};

    outputs.push_back({
        .localBounds      = Rect(0, 0, bounds.w, bounds.h),
        .output           = output,
        .wallpaperSurface = std::make_unique<LayerSurface>(output),
    });

    // outputs[0].wallpaperSurface->paint();
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
        !outputs.empty() ? Rect(0, 0, 0, 0) : outputs[0].output->getBounds();

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

    return false; // don't deletee empty SpanGroups (this makes sense, trust)
}
