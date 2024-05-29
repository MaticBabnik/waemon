#include "WallpaperImage.hh"
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

WallpaperImage::WallpaperImage(const std::string &path) {
    auto img = ImageInput::open(path.c_str());

    if (!img) panic("Image straightup fucked lol");

    const auto &spec = img->spec();

    width  = spec.width;
    height = spec.height;

    auto ystride     = PIXEL_SIZE * width;
    auto colorFormat = getColorFormatString(spec.channelnames);

    pixeldata = new uint32_t[width * height];

    img->read_image(
        0,
        0,
        0,
        4,
        TypeDesc::UINT8,
        pixeldata,
        PIXEL_SIZE,
        ystride,
        ZSTRIDE
    );

    img->close();

    if (!fixColorFormat(colorFormat)) {
        std::print(std::cerr, "Unsupported color format {}!", colorFormat);
    }

    surface = cairo_image_surface_create_for_data(
        (unsigned char *)pixeldata,
        CAIRO_FORMAT_ARGB32,
        width,
        height,
        ystride
    );
}

Vec2<int> WallpaperImage::size() const { return {(int)width, (int)height}; }

WallpaperImage::~WallpaperImage() {
    cairo_surface_destroy(surface);
    delete[] pixeldata;
}
