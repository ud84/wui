//
// Copyright (c) 2021-2023 Anton Golovkov (udattsk at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/ud84/wui
//

#include <wui/graphic/primitive_container.hpp>

#include <wui/common/flag_helpers.hpp>

#include <boost/nowide/convert.hpp>

#ifdef __linux__
#include <xcb/xcb_image.h>
#include <cairo.h>
#include <cairo-xcb.h>
#endif

namespace wui
{

primitive_container::primitive_container(wui::system_context &context__)
    : context_(context__)
{
}

primitive_container::~primitive_container()
{
    release();
}

#ifdef _WIN32

void primitive_container::init()
{
}

void primitive_container::release()
{
    for (auto &p : pens)
    {
        DeleteObject(p.second);
    }
    pens.clear();

    for (auto &b : brushes)
    {
        DeleteObject(b.second);
    }
    brushes.clear();

    for (auto &f : fonts)
    {
        DeleteObject(f.second);
    }
    fonts.clear();
}

HPEN primitive_container::get_pen(int32_t style, int32_t width, color color_)
{
    auto it = pens.find({ { style, width }, color_ });
    if (it != pens.end())
    {
        return it->second;
    }
    auto pen = CreatePen(style, width, color_);

    pens[{ { style, width }, color_ }] = pen;

    return pen;
}

HBRUSH primitive_container::get_brush(color color_)
{
    auto it = brushes.find(color_);
    if (it != brushes.end())
    {
        return it->second;
    }
    auto brush = CreateSolidBrush(color_);

    brushes[color_] = brush;

    return brush;
}

HFONT primitive_container::get_font(font font_)
{
    auto it = fonts.find({ {font_.name, font_.size }, font_.decorations_ });
    if (it != fonts.end())
    {
        return it->second;
    }

    LOGFONTW log_font = { font_.size,
        0,
        0,
        0,
        flag_is_set(font_.decorations_, decorations::bold) ? FW_MEDIUM : FW_DONTCARE,
        flag_is_set(font_.decorations_, decorations::italic),
        flag_is_set(font_.decorations_, decorations::underline),
        flag_is_set(font_.decorations_, decorations::strike_out),
        ANSI_CHARSET,
        OUT_TT_PRECIS,
        CLIP_DEFAULT_PRECIS,
        CLEARTYPE_QUALITY,
        DEFAULT_PITCH | FF_DONTCARE,
        0
    };
    auto font_name = boost::nowide::widen(font_.name);
    memcpy(log_font.lfFaceName, font_name.c_str(), font_name.size() * 2);
    HFONT font__ = CreateFontIndirectW(&log_font);

    fonts[{ {font_.name, font_.size }, font_.decorations_ }] = font__;

    return font__;
}

#elif __linux__

void primitive_container::init()
{
}

void primitive_container::release()
{
    for (auto &g : gcs)
    {
        xcb_free_gc(context_.connection, g.second);
    }
    gcs.clear();

    for (auto &f : fonts)
    {
    	cairo_destroy(f.second);
    }
    fonts.clear();
}

xcb_gcontext_t primitive_container::get_gc(color color_)
{
    auto it = gcs.find(color_);
    if (it != gcs.end())
    {
        return it->second;
    }

    auto gc = xcb_generate_id(context_.connection);

    uint32_t mask = XCB_GC_FOREGROUND;
    uint32_t value[] = { static_cast<uint32_t>(color_) };
    auto gc_create_cookie = xcb_create_gc(context_.connection, gc, context_.wnd, mask, value);

    gcs[color_] = gc;

    return gc;
}

_cairo *primitive_container::get_font(font font_, _cairo_surface *surface)
{
    auto it = fonts.find({ {font_.name, font_.size }, font_.decorations_ });
    if (it != fonts.end())
    {
        return it->second;
    }

    auto cr = cairo_create(surface);
    if (!cr)
    {
        return nullptr;
    }

    cairo_select_font_face(cr, font_.name.c_str(), CAIRO_FONT_SLANT_NORMAL,
        !flag_is_set(font_.decorations_, decorations::bold) ? CAIRO_FONT_WEIGHT_NORMAL : CAIRO_FONT_WEIGHT_NORMAL);
    cairo_set_font_size(cr, font_.size);

    fonts[{ {font_.name, font_.size }, font_.decorations_ }] = cr;

    return cr;
}

#endif

}
