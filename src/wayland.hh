#include <wayland-client.h>
#include <viewporter.h>
#include <xdg-shell.h>
#include <wlr-layer-shell-unstable-v1.h>

struct wlData
{
    int w,h;
    
    wl_display *display;
    wl_registry *registry;

    wl_compositor *compositor;
    wl_shm *shm;
    xdg_wm_base *wm_base;
    xdg_toplevel *toplevel;
    zwlr_layer_shell_v1 *layer_shell;

    wl_surface *wl_surf;
    xdg_surface *xdg_surf;
    zwlr_layer_surface_v1 *ly_surf;
};

void wlmain();