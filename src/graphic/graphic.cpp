//
// Copyright (c) 2021-2022 Anton Golovkov (udattsk at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/ud84/wui
//

#include <wui/graphic/graphic.hpp>
#include <wui/common/flag_helpers.hpp>
#include <wui/system/tools.hpp>

#ifdef __linux__
#include <wui/system/char_encoding.hpp>

#include <cairo.h>
#include <cairo-xcb.h>
#include <cmath>
#endif

namespace wui
{

graphic::graphic(system_context &context__)
    : context_(context__),
      max_size()
#ifdef _WIN32
    , mem_dc(0),
    mem_bitmap(0),
    background_brush(0)
#elif __linux__
    , mem_pixmap(0),
    gc(0),
    surface(nullptr),
    cr(nullptr)
#endif
{
}

graphic::~graphic()
{
    release();
}

void graphic::init(const rect &max_size_, color background_color)
{
#ifdef _WIN32
    if (!context_.dc || mem_dc)
    {
        return;
    }

    max_size = max_size_;

    mem_dc = CreateCompatibleDC(context_.dc);

    HBITMAP mem_bitmap = CreateCompatibleBitmap(context_.dc, max_size.width(), max_size.height());
    SelectObject(mem_dc, mem_bitmap);

    background_brush = CreateSolidBrush(background_color);

    RECT filling_rect = { 0, 0, max_size.width(), max_size.height() };
    FillRect(mem_dc, &filling_rect, background_brush);
#elif __linux__
    if (!context_.connection || mem_pixmap)
    {
        return;
    }

    max_size = max_size_;

    gc = xcb_generate_id(context_.connection);
    uint32_t mask = XCB_GC_FOREGROUND;
    uint32_t value[] = { static_cast<uint32_t>(background_color) };
    auto gc_create_cookie = xcb_create_gc(context_.connection, gc, context_.wnd, mask, value);
    if (!check_cookie(gc_create_cookie, context_.connection, "graphic::init xcb_create_gc"))
    {
        return;
    }

    mem_pixmap = xcb_generate_id(context_.connection);
    auto pixmap_create_cookie = xcb_create_pixmap(context_.connection,
        context_.screen->root_depth,
        mem_pixmap,
        context_.wnd,
        max_size.width(),
        max_size.height());
    if (!check_cookie(pixmap_create_cookie, context_.connection, "graphic::init xcb_create_pixmap"))
    {
        return;
    }

    xcb_rectangle_t rct = { 0,
        0,
        static_cast<uint16_t>(max_size.width()),
        static_cast<uint16_t>(max_size.height()) };
    xcb_poly_fill_rectangle(context_.connection, mem_pixmap, gc, 1, &rct);

    xcb_visualtype_t *visual_type;

    auto depth_iter = xcb_screen_allowed_depths_iterator(context_.screen);
    for (; depth_iter.rem; xcb_depth_next(&depth_iter))
    {
        auto visual_iter = xcb_depth_visuals_iterator(depth_iter.data);
        for (; visual_iter.rem; xcb_visualtype_next(&visual_iter))
        {
            if (context_.screen->root_visual == visual_iter.data->visual_id)
            {
                visual_type = visual_iter.data;
                goto visual_found;
            }
        }
    }
    visual_found: ;

    surface = cairo_xcb_surface_create(context_.connection, mem_pixmap, visual_type, max_size.width(), max_size.height());
    cr = cairo_create(surface);

#endif
}

void graphic::release()
{
#ifdef _WIN32
    DeleteObject(mem_bitmap);
    mem_bitmap = 0;

    DeleteDC(mem_dc);
    mem_dc = 0;

    DeleteObject(background_brush);
    background_brush = 0;
#elif __linux__
    if (mem_pixmap)
    {
        auto free_pixmap_cookie = xcb_free_pixmap(context_.connection, mem_pixmap);
        check_cookie(free_pixmap_cookie, context_.connection, "graphic::clear_resources");
        mem_pixmap = 0;
    }

    if (gc)
    {
        auto free_gc_cookie = xcb_free_gc(context_.connection, gc);
        check_cookie(free_gc_cookie, context_.connection, "graphic::clear_resources");
        gc = 0;
    }

    if (cr)
    {
        cairo_destroy(cr);
        cr = nullptr;
    }
    if (surface)
    {
        cairo_surface_destroy(surface);
        surface = nullptr;
    }

#endif
}

void graphic::set_background_color(color background_color)
{
#ifdef _WIN32
    DeleteObject(background_brush);
    
    background_brush = CreateSolidBrush(background_color);

    RECT filling_rect = { 0, 0, max_size.width(), max_size.height() };
    FillRect(mem_dc, &filling_rect, background_brush);
#elif __linux__
    if (gc)
    {
        auto free_gc_cookie = xcb_free_gc(context_.connection, gc);
        check_cookie(free_gc_cookie, context_.connection, "graphic::clear_resources");
        gc = 0;
    }

    gc = xcb_generate_id(context_.connection);
    uint32_t mask = XCB_GC_FOREGROUND;
    uint32_t value[] = { static_cast<uint32_t>(background_color) };
    auto gc_create_cookie = xcb_create_gc(context_.connection, gc, context_.wnd, mask, value);
    if (!check_cookie(gc_create_cookie, context_.connection, "graphic::set_background_color xcb_create_gc"))
    {
        return;
    }

    xcb_rectangle_t rct = { 0,
        0,
        static_cast<uint16_t>(max_size.width()),
        static_cast<uint16_t>(max_size.height()) };
    xcb_poly_fill_rectangle(context_.connection, mem_pixmap, gc, 1, &rct);
#endif
}

void graphic::clear(const rect &position)
{
#ifdef _WIN32
    if (!context_.dc || !mem_dc)
    {
        return;
    }

    RECT filling_rect = { position.left, position.top, position.right, position.bottom };
    FillRect(mem_dc, &filling_rect, background_brush);
#elif __linux__
    if (!mem_pixmap)
    {
        return;
    }

    xcb_rectangle_t rct = { static_cast<int16_t>(position.left),
        static_cast<int16_t>(position.top),
        static_cast<uint16_t>(position.width()),
        static_cast<uint16_t>(position.height()) };
    xcb_poly_fill_rectangle(context_.connection, mem_pixmap, gc, 1, &rct);
#endif
}

void graphic::flush(const rect &updated_size)
{
#ifdef _WIN32
    if (context_.dc)
    {
        BitBlt(context_.dc,
            updated_size.left,
            updated_size.top,
            updated_size.width(),
            updated_size.height(),
            mem_dc,
            updated_size.left,
            updated_size.top,
            SRCCOPY);
    }
#elif __linux__
    if (context_.wnd)
    {
        auto copy_area_cookie = xcb_copy_area(context_.connection,
            mem_pixmap,
            context_.wnd,
            gc,
            updated_size.left,
            updated_size.top,
            updated_size.left,
            updated_size.top,
            updated_size.width(),
            updated_size.height());

        if (!check_cookie(copy_area_cookie, context_.connection, "graphic::end_drawing xcb_copy_area"))
        {
            return;
        }
    }
#endif
}

void graphic::draw_line(const rect &position, color color_, uint32_t width)
{
#ifdef _WIN32
    auto pen = CreatePen(PS_SOLID, width, color_);
    auto old_pen = (HPEN)SelectObject(mem_dc, pen);
    
    MoveToEx(mem_dc, position.left, position.top, (LPPOINT)NULL);
    LineTo(mem_dc, position.right, position.bottom);

    SelectObject(mem_dc, old_pen);
    DeleteObject(pen);
#elif __linux__
    auto gc_ = xcb_generate_id(context_.connection);

    uint32_t mask = XCB_GC_FOREGROUND;
    uint32_t value[] = { static_cast<uint32_t>(color_) };
    auto gc_create_cookie = xcb_create_gc(context_.connection, gc_, context_.wnd, mask, value);

    xcb_point_t polyline[] = { { static_cast<int16_t>(position.left), static_cast<int16_t>(position.top) },
        { static_cast<int16_t>(position.right), static_cast<int16_t>(position.bottom) } };
    xcb_poly_line(context_.connection, XCB_COORD_MODE_ORIGIN, mem_pixmap, gc_, 2, polyline);

    xcb_free_gc(context_.connection, gc_);
#endif
}

rect graphic::measure_text(const std::wstring &text, const font_settings &font_)
{
#ifdef _WIN32
    HFONT font = CreateFont(font_.size, 0, 0, 0, FW_DONTCARE,
        flag_is_set(font_.decorations, font_decorations::italic),
        flag_is_set(font_.decorations, font_decorations::underline),
        flag_is_set(font_.decorations, font_decorations::strike_out), ANSI_CHARSET,
        OUT_TT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY,
        DEFAULT_PITCH | FF_DONTCARE, font_.name.c_str());

    auto old_font = (HFONT)SelectObject(mem_dc, font);

    RECT text_rect = { 0 };
    DrawTextW(mem_dc, text.c_str(), static_cast<int32_t>(text.size()), &text_rect, DT_CALCRECT);

    SelectObject(mem_dc, old_font);
    DeleteObject(font);

    return rect {0, 0, text_rect.right, text_rect.bottom};
#elif __linux__
    if (!cr)
    {
        return rect{ 0 };
    }
    cairo_text_extents_t extents;

    cairo_select_font_face(cr, to_multibyte(font_.name).c_str(), CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_NORMAL);
    cairo_set_font_size(cr, font_.size - 4);
    cairo_text_extents(cr, to_multibyte(text).c_str(), &extents);

    return rect{ 0, 0, static_cast<int32_t>(ceil(extents.width)), static_cast<int32_t>(ceil(extents.height)) };
#endif
}

void graphic::draw_text(const rect &position, const std::wstring &text, color color_, const font_settings &font_)
{
#ifdef _WIN32
    HFONT font = CreateFont(font_.size, 0, 0, 0, FW_DONTCARE,
        flag_is_set(font_.decorations, font_decorations::italic), 
        flag_is_set(font_.decorations, font_decorations::underline),
        flag_is_set(font_.decorations, font_decorations::strike_out), ANSI_CHARSET,
        OUT_TT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY,
        DEFAULT_PITCH | FF_DONTCARE, font_.name.c_str());

    auto old_font = (HFONT)SelectObject(mem_dc, font);
    
    SetTextColor(mem_dc, color_);
    SetBkMode(mem_dc, TRANSPARENT);

    TextOutW(mem_dc, position.left, position.top, text.c_str(), static_cast<int32_t>(text.size()));

    SelectObject(mem_dc, old_font);
    DeleteObject(font);
#elif __linux__
    cairo_select_font_face(cr, to_multibyte(font_.name).c_str(), CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_NORMAL);
    cairo_set_font_size(cr, font_.size - 4);
    set_color(color_);

    cairo_text_extents_t extents;
    cairo_text_extents(cr, "QWqb", &extents);

    cairo_move_to(cr, position.left, (double)position.top + (extents.height * 3 / 4));
    cairo_show_text(cr, to_multibyte(text).c_str());

    cairo_surface_flush(surface);
#endif
}

void graphic::draw_rect(const rect &position, color fill_color)
{
#ifdef _WIN32
    auto brush = CreateSolidBrush(fill_color);

    RECT position_rect = { position.left, position.top, position.right, position.bottom };
    FillRect(mem_dc, &position_rect, brush);

    DeleteObject(brush);
#elif __linux__
    auto gc_ = xcb_generate_id(context_.connection);

    uint32_t mask = XCB_GC_FOREGROUND;
    uint32_t value[] = { static_cast<uint32_t>(fill_color) };
    auto gc_create_cookie = xcb_create_gc(context_.connection, gc_, context_.wnd, mask, value);

    auto pos = position;
    if (pos.left > pos.right)
    {
        std::swap(pos.left, pos.right);
    }
    if (pos.top > pos.bottom)
    {
        std::swap(pos.top, pos.bottom);
    }

    xcb_rectangle_t rct = { static_cast<int16_t>(pos.left),
        static_cast<int16_t>(pos.top),
        static_cast<uint16_t>(pos.width()),
        static_cast<uint16_t>(pos.height()) };
    xcb_poly_fill_rectangle(context_.connection, mem_pixmap, gc_, 1, &rct);

    xcb_free_gc(context_.connection, gc_);
#endif
}

void graphic::draw_rect(const rect &position, color border_color, color fill_color, uint32_t border_width, uint32_t rnd)
{
#ifdef _WIN32
    auto pen = CreatePen(PS_SOLID, border_width, border_color);
    auto old_pen = (HPEN)SelectObject(mem_dc, pen);

    auto brush = CreateSolidBrush(fill_color);
    auto old_brush = (HBRUSH)SelectObject(mem_dc, brush);

    RoundRect(mem_dc, position.left, position.top, position.right, position.bottom, rnd, rnd);

    SelectObject(mem_dc, old_brush);
    DeleteObject(brush);

    SelectObject(mem_dc, old_pen);
    DeleteObject(pen);
#elif __linux__
    draw_rect(position, fill_color);

    auto gc_ = xcb_generate_id(context_.connection);
    uint32_t mask = XCB_GC_FOREGROUND | XCB_GC_LINE_WIDTH;
    uint32_t value[] = { static_cast<uint32_t>(border_color), border_width };
    auto gc_create_cookie = xcb_create_gc(context_.connection, gc_, context_.wnd, mask, value);

    xcb_rectangle_t rct = { static_cast<int16_t>(position.left),
            static_cast<int16_t>(position.top),
            static_cast<uint16_t>(position.width() - 1),
            static_cast<uint16_t>(position.height() - 1) };

    xcb_poly_rectangle(context_.connection, mem_pixmap, gc_, 1, &rct);

    xcb_free_gc(context_.connection, gc_);
#endif
}

void graphic::draw_buffer(const rect &position, uint8_t *buffer, size_t buffer_size)
{
#ifdef _WIN32
#elif __linux__
#endif
}

void graphic::draw_graphic(const rect &position, graphic &graphic_, int32_t left_shift, int32_t right_shift)
{
#ifdef _WIN32
    if (graphic_.drawable())
    {
        BitBlt(mem_dc,
            position.left,
            position.top,
            position.right,
            position.bottom,
            graphic_.drawable(),
            left_shift,
            right_shift,
            SRCCOPY);
    }
#elif __linux__
    if (graphic_.drawable())
    {
        auto copy_area_cookie = xcb_copy_area(context_.connection,
            graphic_.drawable(),
            mem_pixmap,
            gc,
            left_shift,
            right_shift,
            position.left,
            position.top,
            position.right,
            position.bottom);

        if (!check_cookie(copy_area_cookie, context_.connection, "graphic::draw_graphic xcb_copy_area"))
        {
            return;
        }
    }
#endif
}

#ifdef _WIN32
HDC graphic::drawable()
{
    return mem_dc;
}
#elif __linux__
xcb_drawable_t graphic::drawable()
{
    return mem_pixmap;
}

void graphic::set_color(color color_)
{
    cairo_set_source_rgb(cr,
        static_cast<double>(get_red(color_)) / 255,
        static_cast<double>(get_green(color_)) / 255,
        static_cast<double>(get_blue(color_)) / 255);
}
#endif

}
