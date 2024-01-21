//
// Copyright (c) 2021-2022 Anton Golovkov (udattsk at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/ud84/wui
//

#pragma once

#include <wui/system/system_context.hpp>

#include <wui/common/color.hpp>
#include <wui/common/rect.hpp>
#include <wui/common/font.hpp>
#include <wui/common/error.hpp>

#include <wui/graphic/primitive_container.hpp>

#include <string_view>
#include <cstdint>

#ifdef __linux__
struct _cairo_surface;
struct _cairo_device;
#endif

namespace wui
{

class graphic
{
public:
    graphic(system_context &context);
    ~graphic();

    bool init(const rect &max_size, color background_color);
    void release();

    void set_background_color(color background_color);

    void clear(const rect &position);

    void flush(const rect &updated_size);

    void draw_pixel(const rect &position, color color_);

    void draw_line(const rect &position, color color_, uint32_t width = 1);

    rect measure_text(std::string_view text, const font &font_);
    void draw_text(const rect &position, std::string_view text, color color_, const font &font_);

    void draw_rect(const rect &position, color_alpha fill_color);
    void draw_rect(const rect &position, color border_color, color_alpha fill_color, uint32_t border_width, uint32_t round);

    /// draw some buffer on context
    void draw_buffer(const rect &position, uint8_t *buffer, int32_t left_shift, int32_t top_shift);

    /// draw another graphic on context
    void draw_graphic(const rect &position, graphic &graphic_, int32_t left_shift, int32_t top_shift);

#ifdef _WIN32
    HDC drawable();
#elif __linux__
    xcb_drawable_t drawable();

    /// workarounds on linux
    void start_cairo_device();
    void end_cairo_device();

    void draw_surface(_cairo_surface &surface, const rect &position);
#endif

    error get_error() const;

private:
    system_context &context_;

    primitive_container pc;

    rect max_size;

    color background_color;

#ifdef _WIN32
    HDC mem_dc;
    HBITMAP mem_bitmap;
#elif __linux__
    xcb_pixmap_t mem_pixmap;

    _cairo_surface *surface;
    _cairo_device *device;
#endif

    error err;
};

}
