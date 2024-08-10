#include "WaylandManager.hh"
#include "util/log.hh"
#include "util/panic.hh"
#include <cstring>
#include <iostream>
#include <poll.h>

const wl_registry_listener WaylandManager::REGISTRY_LISTENER = {
    .global        = WaylandManager::ON_REG_GLOBAL,
    .global_remove = WaylandManager::ON_REG_REMOVE,
};

template <typename T>
T *bindGlobal(
    wl_registry        *reg,
    uint32_t            name,
    const wl_interface *iface,
    uint32_t            ver
) {
    return static_cast<T *>(wl_registry_bind(reg, name, iface, ver));
}

/**
 * Disgusting code...
 */
void WaylandManager::ON_REG_GLOBAL(
    void *data,
    wl_registry *,
    uint32_t    name,
    const char *iface,
    uint32_t
) {
    auto that = reinterpret_cast<WaylandManager *>(data);

    if (strcmp(iface, wl_compositor_interface.name) == 0) {
        that->compositor = bindGlobal<wl_compositor>(
            that->registry,
            name,
            &wl_compositor_interface,
            3
        );

    } else if (strcmp(iface, wl_shm_interface.name) == 0) {
        that->shm =
            bindGlobal<wl_shm>(that->registry, name, &wl_shm_interface, 1);

    } else if (strcmp(iface, xdg_wm_base_interface.name) == 0) {
        that->wm_base = bindGlobal<xdg_wm_base>(
            that->registry,
            name,
            &xdg_wm_base_interface,
            1
        );

    } else if (strcmp(iface, zwlr_layer_shell_v1_interface.name) == 0) {
        that->layer_shell = bindGlobal<zwlr_layer_shell_v1>(
            that->registry,
            name,
            &zwlr_layer_shell_v1_interface,
            1
        );

    } else if (strcmp(iface, wl_output_interface.name) == 0) {
        auto output = std::make_shared<WaylandOutput>(that, name);
        that->outputs.emplace(name, output);

    } else if (strcmp(iface, zxdg_output_manager_v1_interface.name) == 0) {
        that->output_manager = bindGlobal<zxdg_output_manager_v1>(
            that->registry,
            name,
            &zxdg_output_manager_v1_interface,
            1
        );
    }
}

void WaylandManager::ON_REG_REMOVE(void *data, wl_registry *, uint32_t name) {
    auto that = reinterpret_cast<WaylandManager *>(data);

    if (that->outputs.contains(name)) {
        that->inputLost(name);
        that->outputs.erase(name);
    }
}

WaylandManager::WaylandManager(const char *display) {
    shm            = nullptr;
    compositor     = nullptr;
    wm_base        = nullptr;
    layer_shell    = nullptr;
    output_manager = nullptr;

    this->display = wl_display_connect(display);
    if (!this->display) panic("Couldn't connect to display.");

    this->registry = wl_display_get_registry(this->display);
    wl_registry_add_listener(this->registry, &REGISTRY_LISTENER, this);

    wl_display_roundtrip(this->display);

    if (!this->compositor) panic("Wayland global is missing(compositor)!");
    if (!this->shm) panic("Wayland global is missing (shm)!");
    if (!this->wm_base) panic("Wayland global is missing (wm_base)!");
    if (!this->layer_shell) panic("Wayland global is missing (layer_shell)!");
    if (!this->output_manager)
        panic("Wayland global is missing (output_manager)!");
}

// TODO: make sure this makes sense
void WaylandManager::dispatch() {
    wl_display_dispatch_pending(display);
    struct pollfd fds[] = {{wl_display_get_fd(display), POLLIN, 0}};
    if (wl_display_prepare_read(display) == 0) {
        wl_display_flush(display);
    }
    int ret = poll(fds, 1, 1); // TODO: make sure this also makes senese

    if (ret > 0 && (fds[0].revents & POLLIN)) {
        wl_display_read_events(display);
        wl_display_dispatch_pending(display);
    } else {
        wl_display_cancel_read(display);
    }
}

void WaylandManager::inputReady(uint32_t wlName) {
    logger::info("Input ready: {}", wlName);
}

void WaylandManager::inputLost(uint32_t wlName) {
    logger::info("Input lost: {}", wlName);
}