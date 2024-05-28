#pragma once
#include <cairo/cairo.h>
#include <functional>
#include <memory>
#include <optional>

#include "WaylandManager.hh"
#include "WaylandOutput.hh"

class LayerSurface {
    using PaintCallback = std::function<void(cairo_t *)>;

  public:
    explicit LayerSurface(std::shared_ptr<WaylandOutput> wo_);
    void paint(PaintCallback callback);

  private:
    WaylandManager                *wm;
    std::shared_ptr<WaylandOutput> wo;

    wl_surface            *surface;
    zwlr_layer_surface_v1 *layer_surf;

    bool configured;

    std::optional<PaintCallback> paintCb;

    const static wl_buffer_listener             BUFFER_LISTENER;
    const static zwlr_layer_surface_v1_listener LAYER_SURF_LISTENER;

    static void ON_LAYER_SURF_CONFIGURE(
        void                  *data,
        zwlr_layer_surface_v1 *surf,
        uint32_t               ser,
        uint32_t               w,
        uint32_t               h
    );
};