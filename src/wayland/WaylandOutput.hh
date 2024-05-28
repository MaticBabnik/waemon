#pragma once

#include <optional>
#include <stdexcept>
#include <string>
#include <wayland-client.h>
#include <xdg-output-unstable-v1.h>

#include "WaylandManager.hh"
#include "util/rect.hh"

class WaylandManager;
class LayerSurface;

class WaylandOutput {
  public:
    WaylandOutput(WaylandManager *wm, uint32_t wl_name);
    ~WaylandOutput();

    bool               valid() const;
    const std::string &getName() const;
    Rect               getBounds() const;
    uint32_t           getWlName() const;

    friend class LayerSurface;

  private:
    bool                       posValid, sizeValid;
    uint32_t                   wl_name;
    std::optional<std::string> name;
    int32_t                   w, h, x, y;

    WaylandManager *wm;
    wl_output      *w_output;
    zxdg_output_v1 *x_output;

    void notify_manager_if_valid();

    const static wl_output_listener      WL_OUTPUT_LISTENER;
    const static zxdg_output_v1_listener XDG_OUTPUT_LISTENER;

    static void ON_WL_OUT_NAME(void *data, wl_output *output, const char *name);
    static void ON_WL_OUT_GEOMETRY(
        void       *data,
        wl_output  *output,
        int32_t     x,
        int32_t     y,
        int32_t     w,
        int32_t     h,
        int32_t     subpx,
        const char *make,
        const char *model,
        int32_t     transform
    );
    static void
    ON_XDG_OUT_POS(void *data, zxdg_output_v1 *output, int32_t x, int32_t y);
    static void
    ON_XDG_OUT_SIZE(void *data, zxdg_output_v1 *output, int32_t w, int32_t h);

    static void ON_WL_OUT_MODE_DUMMY(
        void *,
        wl_output *,
        uint32_t,
        int32_t,
        int32_t,
        int32_t
    );
    static void ON_WL_OUT_DONE_DUMMY(void *, wl_output *);
    static void ON_WL_OUT_SCALE_DUMMY(void *, wl_output *, int32_t);
    static void
    ON_WL_OUT_DESC_DUMMY(void *, wl_output *, const char *description);
    static void ON_XDG_OUT_STRING_DUMMY(
        void           *data,
        zxdg_output_v1 *output,
        const char     *name
    );
    static void ON_XDG_OUT_DONE_DUMMY(void *data, zxdg_output_v1 *output);
};