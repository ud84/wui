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

namespace wui
{

graphic::graphic(system_context &context__)
    : context_(context__),
    draw_position()
#ifdef _WIN32
    , mem_dc(0),
    mem_bitmap(0),
    background_brush(0)
#elif __linux__
    , mem_pixmap(0),
    gc(0)
#endif
{
}

graphic::~graphic()
{
    clear_resources();
}

void graphic::start_drawing(const rect &position, color background_color)
{
#ifdef _WIN32
    if (!context_.dc || mem_dc)
    {
        return;
    }

    draw_position = position;

    background_brush = CreateSolidBrush(background_color);

    mem_dc = CreateCompatibleDC(context_.dc);

    HBITMAP mem_bitmap = CreateCompatibleBitmap(context_.dc, draw_position.width(), draw_position.height());
    SelectObject(mem_dc, mem_bitmap);

    RECT filling_rect = { draw_position.left, draw_position.top, draw_position.right, draw_position.bottom };
    FillRect(mem_dc, &filling_rect, background_brush);
#elif __linux__
    if (!context_.wnd || mem_pixmap)
    {
        return;
    }

    draw_position = position;

    gc = xcb_generate_id(context_.connection);
    uint32_t mask = XCB_GC_FOREGROUND;
    uint32_t value[] = { static_cast<uint32_t>(background_color) };
    auto gc_create_cookie = xcb_create_gc(context_.connection, gc, context_.wnd, mask, value);
    if (!check_cookie(gc_create_cookie, context_.connection, "graphic::start_drawing xcb_create_gc"))
    {
        return;
    }

    auto screen = xcb_setup_roots_iterator(xcb_get_setup(context_.connection)).data;

    mem_pixmap = xcb_generate_id(context_.connection);
    auto pixmap_create_cookie = xcb_create_pixmap(context_.connection,
        screen->root_depth,
		mem_pixmap,
        context_.wnd,
		draw_position.width(),
		draw_position.height());
    if (!check_cookie(pixmap_create_cookie, context_.connection, "graphic::start_drawing xcb_create_pixmap"))
    {
        return;
    }

    xcb_rectangle_t rct = { 0, 0, static_cast<uint16_t>(draw_position.width()), static_cast<uint16_t>(draw_position.height()) };
    xcb_poly_fill_rectangle(context_.connection, mem_pixmap, gc, 1, &rct);
#endif
}

void graphic::end_drawing()
{
#ifdef _WIN32
    if (context_.dc)
    {
        BitBlt(context_.dc,
            draw_position.left,
            draw_position.top,
            draw_position.width(),
            draw_position.height(),
            mem_dc,
            0,
            0,
            SRCCOPY);
    }
#elif __linux__
    if (context_.wnd)
    {
    	auto copy_area_cookie = xcb_copy_area(context_.connection,
            mem_pixmap,
            context_.wnd,
            gc,
            0,
            0,
            draw_position.left,
            draw_position.top,
            draw_position.width(),
            draw_position.height());

        if (!check_cookie(copy_area_cookie, context_.connection, "graphic::end_drawing xcb_copy_area"))
        {
            return;
        }
    }
#endif

    clear_resources();
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

#endif

    return rect{ 0 };
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

    xcb_rectangle_t rct = { static_cast<int16_t>(position.left),
        static_cast<int16_t>(position.top),
	    static_cast<uint16_t>(position.width()),
	    static_cast<uint16_t>(position.height()) };
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
            mem_pixmap,
            context_.wnd,
            gc,
			left_shift,
			right_shift,
            draw_position.left,
            draw_position.top,
            draw_position.width(),
            draw_position.height());

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
#endif

void graphic::clear_resources()
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
#endif
}

}
