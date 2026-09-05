// Copyright (c) 2026 Intent Garden Org. Boost Software License 1.0.
#include "image_wasm.hpp"
#include <emscripten.h>
#include <fstream>
#include <iterator>
EM_JS(int, image_create, (const uint8_t *data, int size), {
    return Module.wui.image(HEAPU8.slice(data, data + size));
});
EM_JS(int, image_dimension, (int id, int axis), {
    var i = Module.wui.images.get(id); return i ? (axis ? i.naturalHeight : i.naturalWidth) : 0;
});
EM_JS(void, image_free, (int id), { Module.wui.images.delete(id); });
void load_image_from_data(const std::vector<uint8_t>& data, void **image)
{
    *image = nullptr;
    if (data.empty()) return;
    int width = 0, height = 0;
    // PNG dimensions are available before the browser finishes decoding.
    if(data.size() >= 24 && data[0] == 137 && data[1] == 80 && data[2] == 78 && data[3] == 71) {
        auto read = [&](int offset) { return (uint32_t(data[offset]) << 24) |
            (uint32_t(data[offset+1]) << 16) | (uint32_t(data[offset+2]) << 8) | data[offset+3]; };
        auto w = read(16), h = read(20);
        if(w <= 32768 && h <= 32768) { width = w; height = h; }
    }
    *image = new wasm_image{image_create(data.data(), data.size()), width, height};
}
void load_image_from_file(std::string_view file, std::string_view path, void **image, wui::error& error)
{
    std::string full = path.empty() ? std::string(file) : std::string(path) + "/" + std::string(file);
    std::ifstream stream(full, std::ios::binary);
    if (!stream) { *image = nullptr; error = {wui::error_type::file_not_found, "image::load_image_from_file()", full}; return; }
    std::vector<uint8_t> data((std::istreambuf_iterator<char>(stream)), {});
    load_image_from_data(data, image);
    if (!*image) error = {wui::error_type::invalid_value, "image::load_image_from_file()", "Empty image: " + full};
    else error.reset();
}
void free_image(void **image)
{
    if(auto p = static_cast<wasm_image *>(*image)) { image_free(p->id); delete p; *image = nullptr; }
}
int wasm_image_width(void *image) { auto p = static_cast<wasm_image *>(image); return p ? (p->width ? p->width : image_dimension(p->id, 0)) : 0; }
int wasm_image_height(void *image) { auto p = static_cast<wasm_image *>(image); return p ? (p->height ? p->height : image_dimension(p->id, 1)) : 0; }
