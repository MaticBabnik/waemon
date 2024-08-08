#include "WallpaperImage.hh"
#include "util/log.hh"
#include "util/panic.hh"
#include <OpenImageIO/imagebuf.h>
#include <algorithm>

constexpr auto PIXEL_SIZE = 4, ZSTRIDE = 1;

using namespace OIIO;

std::string getColorFormatString(const std::vector<std::string> &chans) {
    std::string result = chans.front();
    for (auto it = chans.begin() + 1; it != chans.end(); ++it) {
        result += *it;
    }

    return result;
}

bool WallpaperImage::fixColorFormat(std::string &colorFormat) {
    if (colorFormat == "ARGB") return true;

    if (colorFormat == "RGBA") {
        // Rearrange bytes to ARGB format
        for (size_t i = 0; i < width * height; i++) {
            uint8_t a    = (pixeldata[i] >> 24) & 0xFF;
            uint8_t b    = (pixeldata[i] >> 16) & 0xFF;
            uint8_t g    = (pixeldata[i] >> 8) & 0xFF;
            uint8_t r    = pixeldata[i] & 0xFF;
            pixeldata[i] = (a << 24) | (r << 16) | (g << 8) | b;
        }
        return true;
    }

    if (colorFormat == "RGB") {
        // Add alpha
        for (size_t i = 0; i < width * height; i++) {
            uint8_t b    = (pixeldata[i] >> 16) & 0xFF;
            uint8_t g    = (pixeldata[i] >> 8) & 0xFF;
            uint8_t r    = pixeldata[i] & 0xFF;
            pixeldata[i] = (0xff << 24) | (r << 16) | (g << 8) | b;
        }
        return true;
    }

    return false;
}

WallpaperImage::WallpaperImage(const std::string &path) : path(path) {
    auto img = ImageInput::open(path.c_str());

    if (!img) {
        logger::warn("Could not open '{}'", path);
        this->valid = false;
        return;
    }

    const auto &spec = img->spec();

    width  = spec.width;
    height = spec.height;

    auto ystride     = PIXEL_SIZE * width;
    auto colorFormat = getColorFormatString(spec.channelnames);

    pixeldata = new uint32_t[width * height];

    if (!img->read_image(
            0,
            0,
            0,
            4,
            TypeDesc::UINT8,
            pixeldata,
            PIXEL_SIZE,
            ystride,
            ZSTRIDE
        )) {
        logger::warn("Could not read image @ '{}'", path);

        this->valid = false;
        img->close();
        return;
    }

    img->close();

    if (!fixColorFormat(colorFormat)) {
        logger::warn("Unsupported color format {} @ '{}", colorFormat, path);
    }

    surface = cairo_image_surface_create_for_data(
        (unsigned char *)pixeldata,
        CAIRO_FORMAT_ARGB32,
        width,
        height,
        ystride
    );
}

bool WallpaperImage::isValid() const { return this->valid; }

Vec2<int> WallpaperImage::size() const { return {(int)width, (int)height}; }

cairo_surface_t *WallpaperImage::getSurface() { return this->surface; }

std::string WallpaperImage::getPath() { return this->path; }

WallpaperImage::~WallpaperImage() {
    if (surface) cairo_surface_destroy(surface);
    if (pixeldata) delete[] pixeldata;
}

std::map<std::string, std::weak_ptr<WallpaperImage>> WallpaperCache::cache;
std::shared_ptr<WallpaperImage> WallpaperCache::get(const std::string &path) {
    if (cache.contains(path)) {
        auto cached = cache.at(path).lock();
        if (cached) {
            logger::info("Cached '{}'", path);
            return {cached};
        }
    }

    logger::info("Loading '{}'", path);
    auto wp = new WallpaperImage(path);

    if (!wp->isValid()) {
        delete wp;
        return nullptr;
    }

    // clean expired pointers
    std::erase_if(cache, [](const auto &pair) {
        return pair.second.expired();
    });

    std::shared_ptr<WallpaperImage> sp{wp};
    // Store a weak reference
    cache.insert({path, sp});
    return sp;
}