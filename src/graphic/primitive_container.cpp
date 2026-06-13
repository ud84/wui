//
// Copyright (c) 2021-2026 Intent Garden Org
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
//

#include <wui/graphic/primitive_container.hpp>

#include <boost/nowide/convert.hpp>

#ifdef _WIN32
#include <strsafe.h>
#elif __linux__
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
    err.reset();
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

    for (auto &b : bitmaps)
    {
        DeleteObject(b.second);
    }
    bitmaps.clear();
}

HPEN primitive_container::get_pen(const int32_t style, const int32_t width, const color color_)
{
    auto it = pens.find({ { style, width }, color_ });
    if (it != pens.end())
    {
        return it->second;
    }
    auto pen = CreatePen(style, width, get_rgb(color_));

    pens[{ { style, width }, color_ }] = pen;

    return pen;
}

HBRUSH primitive_container::get_brush(const color color_)
{
    if (is_alpha(color_)) // TODO: alpha
    {
        return (HBRUSH)GetStockObject(NULL_BRUSH);
    }
    auto it = brushes.find(color_);
    if (it != brushes.end())
    {
        return it->second;
    }

    auto brush = CreateSolidBrush(get_rgb(color_));

    brushes[color_] = brush;

    return brush;
}

HFONT primitive_container::get_font(const font& font_)
{
    auto it = fonts.find({ {font_.name, font_.size }, font_.decorations_ });
    if (it != fonts.end())
    {
        return it->second;
    }

    LOGFONTW log_font = {
        font_.size,
        0,
        0,
        0,
        (font_.decorations_ & decorations::bold) ? FW_MEDIUM : FW_DONTCARE,
        0 != (font_.decorations_ & decorations::italic),
        0 != (font_.decorations_ & decorations::underline),
        0 != (font_.decorations_ & decorations::strike_out),
        ANSI_CHARSET,
        OUT_TT_PRECIS,
        CLIP_DEFAULT_PRECIS,
        CLEARTYPE_QUALITY,
        DEFAULT_PITCH | FF_DONTCARE,
        0
    };
    std::wstring font_name = boost::nowide::widen(font_.name);
#if 0
    const size_t length = font_name.length() < LF_FACESIZE ? font_name.length() : LF_FACESIZE - 1;
    std::memcpy(log_font.lfFaceName, font_name.data(), length * sizeof(WCHAR));
    log_font.lfFaceName[length] = 0x0000;
#else
    StringCchCopyW(log_font.lfFaceName, LF_FACESIZE, font_name.c_str());
#endif
    HFONT font__ = CreateFontIndirectW(&log_font);

    fonts[{ {font_.name, font_.size }, font_.decorations_ }] = font__;

    return font__;
}

HBITMAP primitive_container::get_bitmap(const int32_t width, const int32_t height,
    uint8_t *buffer, const HDC hdc)
{
    auto it = bitmaps.find({ width, height });
    if (it != bitmaps.end())
    {
        auto bitmap = it->second;

        BITMAPINFO bmpInfo;

        bmpInfo.bmiHeader.biSize = sizeof(BITMAPINFO) - sizeof(RGBQUAD);
        bmpInfo.bmiHeader.biWidth = width;
        bmpInfo.bmiHeader.biHeight = 0 - (LONG)height;
        bmpInfo.bmiHeader.biPlanes = 1;
        bmpInfo.bmiHeader.biBitCount = 32;
        bmpInfo.bmiHeader.biCompression = BI_RGB;
        bmpInfo.bmiHeader.biSizeImage = 0;
        bmpInfo.bmiHeader.biXPelsPerMeter = 0;
        bmpInfo.bmiHeader.biYPelsPerMeter = 0;
        bmpInfo.bmiHeader.biClrUsed = 0;
        bmpInfo.bmiHeader.biClrImportant = 0;

        SetDIBits(hdc, bitmap, 0, height, buffer, &bmpInfo, DIB_RGB_COLORS);

        return bitmap;
    }

    auto bitmap = CreateBitmap(width, height, 1, 32, buffer);

    bitmaps[{ width, height }] = bitmap;

    return bitmap;
}

#elif __linux__

void primitive_container::init()
{
    err.reset();
}

void primitive_container::release()
{
    if (context_.connection)
    {
        for (auto &g : gcs)
        {
            xcb_free_gc(context_.connection, g.second);
        }
        gcs.clear();
    }

    for (auto &f : fonts)
    {
    	cairo_destroy(f.second);
    }
    fonts.clear();
}

xcb_gcontext_t primitive_container::get_gc(const color color_)
{
    if (!context_.connection)
    {
        err.set(error_type::no_handle, "primitive_container::get_gc(color)", "no context_.connection");
        return -1;
    }

    auto it = gcs.find(color_);
    if (it != gcs.end())
    {
        return it->second;
    }

    auto gc = xcb_generate_id(context_.connection);

    uint32_t mask = XCB_GC_FOREGROUND;
    uint32_t value[] = { color_ };
    auto gc_create_cookie = xcb_create_gc(context_.connection, gc, context_.wnd, mask, value);

    gcs[color_] = gc;

    return gc;
}

_cairo *primitive_container::get_font(const font& font_, _cairo_surface *surface)
{
    if (!surface)
    {
        return nullptr;
    }

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

    cairo_select_font_face(cr, font_.name.c_str(),
        !(font_.decorations_ & decorations::italic) ? CAIRO_FONT_SLANT_NORMAL : CAIRO_FONT_SLANT_ITALIC,
        !(font_.decorations_ & decorations::bold) ? CAIRO_FONT_WEIGHT_NORMAL : CAIRO_FONT_WEIGHT_BOLD);
    cairo_set_font_size(cr, font_.size);

    fonts[{ {font_.name, font_.size }, font_.decorations_ }] = cr;

    return cr;
}

#endif

wui::error primitive_container::get_error() const
{
    return err;
}

}
