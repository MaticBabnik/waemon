#pragma once

#include <map>
#include <memory>

#include <wayland-client.h>
#include <wlr-layer-shell-unstable-v1.h>
#include <xdg-output-unstable-v1.h>
#include <xdg-shell.h>

#include "WaylandOutput.hh"

class WaylandOutput;
class LayerSurface;

class WaylandManager {
  public:
    WaylandManager(const char *display = nullptr);

    void dispatch();

    friend class WaylandOutput;
    friend class LayerSurface;

  protected:
    std::map<uint32_t, std::shared_ptr<WaylandOutput>> outputs;

    wl_display  *display;
    wl_registry *registry;

    wl_compositor          *compositor;
    wl_shm                 *shm;
    xdg_wm_base            *wm_base;
    zwlr_layer_shell_v1    *layer_shell;
    zxdg_output_manager_v1 *output_manager;

    virtual void inputReady(uint32_t wlName);
    virtual void inputLost(uint32_t wlName);

    const static wl_registry_listener REGISTRY_LISTENER;

    static void ON_REG_GLOBAL(
        void        *data,
        wl_registry *reg,
        uint32_t     name,
        const char  *interface,
        uint32_t     ver
    );

    static void ON_REG_REMOVE(void *data, wl_registry *reg, uint32_t name);
};