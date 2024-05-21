#include "wayland.hh"
#include "panic.hh"

#include <iostream>
#include <stdio.h>
#include <string.h>
#include <syscall.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <fcntl.h>

#include <string>

#include "old/font.h"

bool set_cloexec(int fd)
{
    long flags = fcntl(fd, F_GETFD);
    if (flags == -1)
        return false;

    return fcntl(fd, F_SETFD, flags | FD_CLOEXEC) != -1;
}

int make_tmpfile(size_t size)
{
    auto rtdir = getenv("XDG_RUNTIME_DIR");
    if (!rtdir)
        panic("No XDG_RUNTIME_DIR");

    auto tmpname = std::string(rtdir) + "/paper_XXXXXX";

    auto fd = mkstemp((char *)tmpname.c_str());

    if (fd == -1)
        panic("mkstemp failed");

    if (!set_cloexec(fd))
        panic("cloexec failed");

    if (ftruncate(fd, size) < 0)
        panic("ftruncate failed");

    return fd;
}

static void wl_buffer_release(void *data, struct wl_buffer *wl_buffer)
{
    wl_buffer_destroy(wl_buffer);
}

static const struct wl_buffer_listener wl_buffer_listener = {
    .release = wl_buffer_release,
};

const char text[] = "Wallpaper? I hardly even know her!";
constexpr auto tl = sizeof(text) - 1;

static struct wl_buffer *draw_frame(struct wlData *data)
{
    const int width = data->w, height = data->h, stride = width * 4, size = stride * height;

    int fd = make_tmpfile(size);
    uint32_t *buf = (uint32_t *)mmap(nullptr, size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);

    if (buf == MAP_FAILED)
        panic("mmap failed");

    struct wl_shm_pool *pool = wl_shm_create_pool(data->shm, fd, size);
    struct wl_buffer *buffer = wl_shm_pool_create_buffer(pool, 0, width, height, stride, WL_SHM_FORMAT_ARGB8888);
    wl_shm_pool_destroy(pool);
    close(fd);

    for (int y = 0; y < height; ++y)
        for (int x = 0; x < width; ++x)
            buf[y * width + x] = 0xff000000 | (y >> 3) | ((x >> 4) << 16);

    for (int i = 0; i < tl; i++)
    {
        char c = text[i];
        for (int cy = 0; cy < 8; cy++)
            for (int cx = 0; cx < 8; cx++)
                if ((font8x8_basic[c][cy] & (1 << cx)))
                {
                    buf[(cy * 2 + 700) * width + i * 16 + cx * 2 + 1300] = 0xffffffff;
                    buf[(cy * 2 + 701) * width + i * 16 + cx * 2 + 1300] = 0xffffffff;
                    buf[(cy * 2 + 700) * width + i * 16 + cx * 2 + 1301] = 0xffffffff;
                    buf[(cy * 2 + 701) * width + i * 16 + cx * 2 + 1301] = 0xffffffff;
                }
    }

    munmap(buf, size);
    wl_buffer_add_listener(buffer, &wl_buffer_listener, nullptr);
    return buffer;
}

void registry_global_handler(void *d_, wl_registry *registry, uint32_t name, const char *interface, uint32_t version)
{
    auto data = reinterpret_cast<wlData *>(d_);

    if (strcmp(interface, "wl_compositor") == 0)
        data->compositor = (wl_compositor *)wl_registry_bind(registry, name, &wl_compositor_interface, 3);
    else if (strcmp(interface, "wl_shm") == 0)
        data->shm = (wl_shm *)wl_registry_bind(registry, name, &wl_shm_interface, 1);
    else if (strcmp(interface, "xdg_wm_base") == 0)
        data->wm_base = (xdg_wm_base *)wl_registry_bind(registry, name, &xdg_wm_base_interface, 1);
    else if (strcmp(interface, zwlr_layer_shell_v1_interface.name) == 0)
        data->layer_shell = (zwlr_layer_shell_v1 *)wl_registry_bind(registry, name, &zwlr_layer_shell_v1_interface, 1);
}

static void registry_global_remove_handler(void *d_, wl_registry *registry, uint32_t name)
{
}

static void xdg_surface_configure(void *d_, struct xdg_surface *xdg_surface, uint32_t serial)
{
    auto data = reinterpret_cast<wlData *>(d_);
    xdg_surface_ack_configure(data->xdg_surf, serial);

    struct wl_buffer *buffer = draw_frame(data);
    wl_surface_attach(data->wl_surf, buffer, 0, 0);
    wl_surface_commit(data->wl_surf);
}

static void xdg_wm_base_ping(void *data, struct xdg_wm_base *base, uint32_t s)
{
    xdg_wm_base_pong(base, s);
}

const struct xdg_surface_listener xdg_surface_listener = {
    .configure = xdg_surface_configure,
};

static void layer_surface_configure(void *d_, zwlr_layer_surface_v1 *surf, uint32_t s, uint32_t w, uint32_t h)
{
    auto data = reinterpret_cast<wlData *>(d_);

    data->w = w;
    data->h = h;

    zwlr_layer_surface_v1_ack_configure(surf, s);

    struct wl_buffer *buffer = draw_frame(data);
    wl_surface_attach(data->wl_surf, buffer, 0, 0);
    wl_surface_commit(data->wl_surf);
}

const struct zwlr_layer_surface_v1_listener layer_surface_listener = {
    .configure = layer_surface_configure,
};

const wl_registry_listener registry_listener = {
    .global = registry_global_handler,
    .global_remove = registry_global_remove_handler,
};

const struct xdg_wm_base_listener base_listener = {
    .ping = xdg_wm_base_ping};

constexpr wl_output *default_output = nullptr;

void wlmain()
{
    wlData data = {0};

    data.display = wl_display_connect(NULL);
    data.registry = wl_display_get_registry(data.display);
    wl_registry_add_listener(data.registry, &registry_listener, &data);

    wl_display_roundtrip(data.display);

    data.wl_surf = wl_compositor_create_surface(data.compositor);

    data.ly_surf = zwlr_layer_shell_v1_get_layer_surface(
        data.layer_shell,
        data.wl_surf,
        default_output,
        ZWLR_LAYER_SHELL_V1_LAYER_BACKGROUND,
        "paper");

    zwlr_layer_surface_v1_set_anchor(data.ly_surf, ZWLR_LAYER_SURFACE_V1_ANCHOR_TOP |
                                                       ZWLR_LAYER_SURFACE_V1_ANCHOR_BOTTOM |
                                                       ZWLR_LAYER_SURFACE_V1_ANCHOR_LEFT |
                                                       ZWLR_LAYER_SURFACE_V1_ANCHOR_RIGHT);

    zwlr_layer_surface_v1_set_exclusive_zone(data.ly_surf, -1);

    zwlr_layer_surface_v1_add_listener(data.ly_surf, &layer_surface_listener, &data);

    wl_surface_commit(data.wl_surf);

    while (wl_display_dispatch(data.display))
    {
    }
}