//
// Copyright (c) 2021-2023 Anton Golovkov (udattsk at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/ud84/wui
//

#pragma once

#include <wui/common/color.hpp>
#include <wui/common/rect.hpp>
#include <wui/common/font.hpp>
#include <wui/system/system_context.hpp>

#include <map>

#ifdef _WIN32
#include <windows.h>
#elif __linux__
struct _cairo;
struct _cairo_surface;
#endif

namespace wui
{

class primitive_container
{
public:
    primitive_container(wui::system_context &context_);
    ~primitive_container();

    void init();
    void release();

#ifdef _WIN32
    HPEN get_pen(int32_t style, int32_t width, color color_);
    HBRUSH get_brush(color color_);
    HFONT get_font(font font_);
#elif __linux__
    xcb_gcontext_t get_gc(color color_);
    _cairo *get_font(font font_, _cairo_surface *surface);
#endif

private:
    wui::system_context &context_;

#ifdef _WIN32
    std::map<std::pair<std::pair<int32_t, int32_t>, color>, HPEN> pens;
    std::map<int32_t, HBRUSH> brushes;
    std::map<font, HFONT> fonts;
#elif __linux__
    std::map<color, xcb_gcontext_t> gcs;
    std::map<font, _cairo*> fonts;
#endif
};

}
