#include "Group.hh"
#include <algorithm>
#include <iostream>
#include <ranges>
#include <utility>

void BaseWallpaperGroup::setWallpaper(const std::shared_ptr<WallpaperImage> &img
) {
    this->wallpaper = img;
    applyWallpaper();
}

void BaseWallpaperGroup::setFillColor(const Color &c) { this->fill_color = c; }

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

        o.wallpaperSurface->paint([wp, r, mode, fill, br, on](cairo_t *cr) {
            // paint background color
            cairo_set_source_rgb(
                cr,
                fill.r / 255.0,
                fill.g / 255.0,
                fill.b / 255.0
            );
            cairo_paint(cr);

            // special, cancer case
            if (mode == DisplayMode::Tile) {
                cairo_matrix_t m;
                auto           ox = r.x % wp->size().x, oy = r.y % wp->size().y;

                auto p = cairo_pattern_create_for_surface(wp->getSurface());
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
                wp->getSurface(),
                outputSpacePos.x / sx, // scale into cairo space
                outputSpacePos.y / sy
            );

            cairo_paint(cr);
        });
    }
}
