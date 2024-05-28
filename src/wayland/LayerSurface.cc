#include "LayerSurface.hh"
#include "util/panic.hh"
#include <fcntl.h>
#include <iostream>
#include <sys/mman.h>
#include <sys/types.h>
#include <syscall.h>
#include <unistd.h>

static void release_buffer(void *data, struct wl_buffer *wl_buffer) {
    wl_buffer_destroy(wl_buffer);
}

const wl_buffer_listener LayerSurface::BUFFER_LISTENER = {
    .release = release_buffer,
};

const zwlr_layer_surface_v1_listener LayerSurface::LAYER_SURF_LISTENER = {
    .configure = LayerSurface::ON_LAYER_SURF_CONFIGURE,
    .closed    = nullptr, // FIXME: I hope this doesn't blow up
};

int make_tmpfile(size_t size) {
    auto rtdir = getenv("XDG_RUNTIME_DIR");
    if (!rtdir) panic("No XDG_RUNTIME_DIR");

    auto tmpname = std::string(rtdir) + "/paper_XXXXXX";

    auto fd = mkostemp((char *)tmpname.c_str(), O_CLOEXEC);

    if (fd == -1) panic("mkostemp failed");

    if (unlink(tmpname.c_str()) != 0)
        std::print(std::cout, "Couldn't unlink tmpfile\n");

    if (ftruncate(fd, size) < 0) panic("ftruncate failed");

    return fd;
}

LayerSurface::LayerSurface(std::shared_ptr<WaylandOutput> wo_)
    : wm(wo_->wm), wo(wo_) {
    surface    = wl_compositor_create_surface(wm->compositor);
    layer_surf = zwlr_layer_shell_v1_get_layer_surface(
        wm->layer_shell,
        surface,
        wo->w_output,
        ZWLR_LAYER_SHELL_V1_LAYER_BACKGROUND,
        "paper"
    );

    zwlr_layer_surface_v1_set_anchor(
        layer_surf,
        ZWLR_LAYER_SURFACE_V1_ANCHOR_TOP | ZWLR_LAYER_SURFACE_V1_ANCHOR_BOTTOM
            | ZWLR_LAYER_SURFACE_V1_ANCHOR_LEFT
            | ZWLR_LAYER_SURFACE_V1_ANCHOR_RIGHT
    );

    zwlr_layer_surface_v1_set_exclusive_zone(layer_surf, -1);

    zwlr_layer_surface_v1_add_listener(layer_surf, &LAYER_SURF_LISTENER, this);

    wl_surface_commit(surface);
}

void LayerSurface::paint() {
    auto b      = wo->getBounds();
    auto stride = b.w * 4, size = stride * b.h;

    auto fd = make_tmpfile(size);

    auto *buf = (uint32_t *)
        mmap(nullptr, size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);

    if (buf == MAP_FAILED) panic("mmap failed");

    auto pool  = wl_shm_create_pool(wm->shm, fd, size);
    auto wlbuf = wl_shm_pool_create_buffer(
        pool,
        0,
        b.w,
        b.h,
        stride,
        WL_SHM_FORMAT_ABGR8888
    );

    wl_shm_pool_destroy(pool);
    close(fd);

    for (auto i = 0; i < b.w * b.h; i++)
        buf[i] = 0xffff0000;

    munmap(buf, size);

    wl_buffer_add_listener(wlbuf, &BUFFER_LISTENER, nullptr);
    wl_surface_attach(surface, wlbuf, 0, 0);
    wl_surface_commit(surface);
}

void LayerSurface::ON_LAYER_SURF_CONFIGURE(
    void                  *data,
    zwlr_layer_surface_v1 *surf,
    uint32_t               ser,
    uint32_t               w,
    uint32_t               h
) {
    auto that = reinterpret_cast<LayerSurface *>(data);

    zwlr_layer_surface_v1_ack_configure(surf, ser);
    that->paint();
}