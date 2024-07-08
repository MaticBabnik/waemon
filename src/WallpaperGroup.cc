#include "WallpaperGroup.hh"
#include <algorithm>
#include <iostream>
#include <ranges>
#include <utility>

void BaseWallpaperGroup::setWallpaper(const std::shared_ptr<WallpaperImage> &img
) {
    this->wallpaper = img;
    applyWallpaper();
}

void BaseWallpaperGroup::setFillColor(uint8_t r, uint8_t g, uint8_t b) {
    this->fill_color = {r, g, b};
}

void BaseWallpaperGroup::setDisplayMode(DisplayMode dm) {
    this->displayMode = dm;
}

void BaseWallpaperGroup::applyWallpaper() {
    if (!wallpaper.has_value()) return;

    for (auto &o : outputs) {
        auto wp   = this->wallpaper->get();
        auto r    = o.localBounds;
        auto mode = displayMode;
        auto fill = fill_color;
        auto br   = bounds;
        auto on   = o.output->getName();

        std::print(std::cout, "Trying to paint to: {}\n", on);

        o.wallpaperSurface->paint([wp, r, mode, fill, br, on](cairo_t *cr) {
            std::print(
                std::cout,
                "Painting to: {}, offset: {},{}\n",
                on,
                r.x,
                r.y
            );

            // paint background color
            cairo_set_source_rgb(
                cr,
                std::get<0>(fill) / 255.0,
                std::get<1>(fill) / 255.0,
                std::get<2>(fill) / 255.0
            );
            cairo_paint(cr);

            // special, cancer case
            if (mode == DisplayMode::Tile) {
                cairo_matrix_t m;
                auto           ox = r.x % wp->size().x, oy = r.y % wp->size().y;

                auto p = cairo_pattern_create_for_surface(wp->surface);
                cairo_pattern_set_extend(p, CAIRO_EXTEND_REPEAT);

                if (ox || oy) { // apply offset matrix if nonzero
                    cairo_matrix_init_translate(&m, ox, oy);
                    cairo_pattern_set_matrix(p, &m); // matriks blyat
                }

                cairo_set_source(cr, p);
                cairo_paint(cr);
                cairo_pattern_destroy(p);
                return;
            }

            // Center, Zoom, Contain, Stretch:
            double sx = (double)br.w / (double)wp->size().x,
                   sy = (double)br.h / (double)wp->size().y;

            if (mode == DisplayMode::Center) sy = sx = 1;
            else if (mode == DisplayMode::Zoom) sy = sx = std::max(sx, sy);
            else if (mode == DisplayMode::Contain) sy = sx = std::min(sx, sy);

            // Math in groupSpace (in pixels, relative to WallpaperGroup):
            Vec2<double> groupSz{br.size()};
            Vec2<double> wallpaperSz{wp->size().x * sx, wp->size().y * sy};
            Vec2<double> groupSpacePos = (groupSz - wallpaperSz) / 2.0;

            // Offset to output/screen space
            auto outputSpacePos = groupSpacePos - (Vec2<double>)r.origin();

            cairo_scale(cr, sx, sy);
            cairo_set_source_surface(
                cr,
                wp->surface,
                outputSpacePos.x / sx, // scale into cairo space
                outputSpacePos.y / sy
            );

            cairo_paint(cr);
        });
    }
}

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

    // outputs[0].wallpaperSurface->paint();
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

    std::print(
        std::cout,
        "{}: {}x{}@{},{}\n",
        name,
        bounds.w,
        bounds.h,
        bounds.x,
        bounds.y
    );

    // make localBounds local
    for (auto &o : outputs) {
        auto b = o.output->getBounds();

        o.localBounds.x = b.x - boundingBox.x;
        o.localBounds.y = b.y - boundingBox.y;
        std::print(
            std::cout,
            "\t{}: {}x{}@{},{}\n",
            o.output->getName(),
            o.localBounds.w,
            o.localBounds.h,
            o.localBounds.x,
            o.localBounds.y
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

    recomputeBounds();
    if (!outputs.empty()) {
        applyWallpaper();
    }

    return false;
}
