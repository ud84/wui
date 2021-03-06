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

#include <boost/nowide/convert.hpp>

#ifdef __linux__
#include <xcb/xcb_image.h>

#include <cairo.h>
#include <cairo-xcb.h>
#include <cmath>

#include <algorithm>

xcb_visualtype_t *default_visual_type(wui::system_context &context_)
{
    auto depth_iter = xcb_screen_allowed_depths_iterator(context_.screen);
    for (; depth_iter.rem; xcb_depth_next(&depth_iter))
    {
        auto visual_iter = xcb_depth_visuals_iterator(depth_iter.data);
        for (; visual_iter.rem; xcb_visualtype_next(&visual_iter))
        {
            if (context_.screen->root_visual == visual_iter.data->visual_id)
            {
                return visual_iter.data;
            }
        }
    }
    return nullptr;
}

void set_color(cairo_t *cr, wui::color color_)
{
    cairo_set_source_rgb(cr,
        static_cast<double>(wui::get_red(color_)) / 255,
        static_cast<double>(wui::get_green(color_)) / 255,
        static_cast<double>(wui::get_blue(color_)) / 255);
}

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
    device(nullptr)
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

    SetMapMode(mem_dc, MM_TEXT);

    background_brush = CreateSolidBrush(background_color);

    RECT filling_rect = { 0, 0, max_size.width(), max_size.height() };
    FillRect(mem_dc, &filling_rect, background_brush);
#elif __linux__
    if (!context_.display || mem_pixmap)
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

    surface = cairo_xcb_surface_create(context_.connection, mem_pixmap, default_visual_type(context_), max_size.width(), max_size.height());
    if (!surface)
    {
        fprintf(stderr, "WUI error can't create the cairo surface on graphic::init()");
    }
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
    if (surface)
    {
        cairo_surface_destroy(surface);
        surface = nullptr;
    }

    if (gc)
    {
        auto free_gc_cookie = xcb_free_gc(context_.connection, gc);
        check_cookie(free_gc_cookie, context_.connection, "graphic::clear_resources");
        gc = 0;
    }

    if (mem_pixmap)
    {
        auto free_pixmap_cookie = xcb_free_pixmap(context_.connection, mem_pixmap);
        check_cookie(free_pixmap_cookie, context_.connection, "graphic::clear_resources");
        mem_pixmap = 0;
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
    if (!context_.display || !mem_pixmap)
    {
        return;
    }

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

void graphic::draw_pixel(const rect &position, color color_)
{
#ifdef _WIN32
    SetPixel(mem_dc, position.left, position.top, color_);
#elif __linux__
    auto gc_ = xcb_generate_id(context_.connection);

    uint32_t mask = XCB_GC_FOREGROUND;
    uint32_t value[] = { static_cast<uint32_t>(color_) };
    auto gc_create_cookie = xcb_create_gc(context_.connection, gc_, context_.wnd, mask, value);

    xcb_point_t points[] = { { static_cast<int16_t>(position.left), static_cast<int16_t>(position.top) } };

    xcb_poly_point(context_.connection, XCB_COORD_MODE_ORIGIN, mem_pixmap, gc_, 1, points);

    xcb_free_gc(context_.connection, gc_);
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

rect graphic::measure_text(const std::string &text, const font &font__)
{
#ifdef _WIN32
    HFONT font_ = CreateFont(font__.size, 0, 0, 0, FW_DONTCARE,
        flag_is_set(font__.decorations_, decorations::italic),
        flag_is_set(font__.decorations_, decorations::underline),
        flag_is_set(font__.decorations_, decorations::strike_out), ANSI_CHARSET,
        OUT_TT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY,
        DEFAULT_PITCH | FF_DONTCARE, boost::nowide::widen(font__.name).c_str());

    auto old_font = (HFONT)SelectObject(mem_dc, font_);

    RECT text_rect = { 0 };
    auto wide_str = boost::nowide::widen(text);
    DrawTextW(mem_dc, wide_str.c_str(), static_cast<int32_t>(wide_str.size()), &text_rect, DT_CALCRECT);

    SelectObject(mem_dc, old_font);
    DeleteObject(font_);

    return {0, 0, text_rect.right, text_rect.bottom};
#elif __linux__
    if (!surface)
    {
        fprintf(stderr, "WUI error no cairo on graphic::measure_text()");
        return rect{ 0 };
    }

    auto text_ = text; /// workaround to correct measure spaces
    std::replace(text_.begin(), text_.end(), ' ', 'i');

    auto cr = cairo_create(surface);
    if (!cr)
    {
        fprintf(stderr, "WUI error: no cairo context on graphic::measure_text\n");
        return rect{ 0 };
    }
    cairo_text_extents_t extents;

    cairo_select_font_face(cr, font__.name.c_str(), CAIRO_FONT_SLANT_NORMAL,
        !flag_is_set(font__.decorations_, decorations::bold) ? CAIRO_FONT_WEIGHT_NORMAL : CAIRO_FONT_WEIGHT_NORMAL);
    cairo_set_font_size(cr, font__.size);
    cairo_text_extents(cr, text_.c_str(), &extents);

    cairo_destroy(cr);

    return { 0, 0, static_cast<int32_t>(ceil(extents.width)), static_cast<int32_t>(ceil(extents.height)) };
#endif
}

void graphic::draw_text(const rect &position, const std::string &text, color color_, const font &font__)
{
#ifdef _WIN32
    HFONT font_ = CreateFont(font__.size, 0, 0, 0, flag_is_set(font__.decorations_, decorations::bold) ? FW_BOLD : FW_DONTCARE,
        flag_is_set(font__.decorations_, decorations::italic), 
        flag_is_set(font__.decorations_, decorations::underline),
        flag_is_set(font__.decorations_, decorations::strike_out), ANSI_CHARSET,
        OUT_TT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY,
        DEFAULT_PITCH | FF_DONTCARE, boost::nowide::widen(font__.name).c_str());

    auto old_font = (HFONT)SelectObject(mem_dc, font_);
    
    SetTextColor(mem_dc, color_);
    SetBkMode(mem_dc, TRANSPARENT);

    auto wide_str = boost::nowide::widen(text);
    TextOutW(mem_dc, position.left, position.top, wide_str.c_str(), static_cast<int32_t>(wide_str.size()));

    SelectObject(mem_dc, old_font);
    DeleteObject(font_);
#elif __linux__
    if (!surface)
    {
        fprintf(stderr, "WUI error no cairo on graphic::draw_text()");
        return;
    }

    auto cr = cairo_create(surface);

    if (!cr)
    {
        fprintf(stderr, "WUI error: no cairo context on graphic::draw_text\n");
        return;
    }

    set_color(cr, color_);

    cairo_select_font_face(cr, font__.name.c_str(), CAIRO_FONT_SLANT_NORMAL,
        !flag_is_set(font__.decorations_, decorations::bold) ? CAIRO_FONT_WEIGHT_NORMAL : CAIRO_FONT_WEIGHT_NORMAL);
    cairo_set_font_size(cr, font__.size);

    cairo_move_to(cr, position.left, (double)position.top + font__.size * 5 / 6);
    cairo_show_text(cr, text.c_str());

    cairo_destroy(cr);
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
    auto pen = CreatePen(border_width != 0 ? PS_SOLID : PS_NULL, border_width, border_color);
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

    if (border_width != 0)
    {
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
    }
#endif
}

void graphic::draw_buffer(const rect &position, uint8_t *buffer, int32_t left_shift, int32_t top_shift)
{
#ifdef _WIN32
    HBITMAP source_bitmap = CreateBitmap(position.width(), position.height(), 1, 32, buffer);
    auto source_dc = CreateCompatibleDC(mem_dc);
    SelectObject(source_dc, source_bitmap);

    BitBlt(mem_dc,
        position.left,
        position.top,
        position.width(),
        position.height(),
        source_dc,
        left_shift,
        top_shift,
        SRCCOPY);

    DeleteObject(source_bitmap);
    DeleteDC(source_dc);
#elif __linux__
    auto pixmap = xcb_generate_id(context_.connection);
    auto pixmap_cookie = xcb_create_pixmap(context_.connection,
        context_.screen->root_depth,
		pixmap,
		context_.wnd,
		position.width(), position.height());

    if (!check_cookie(pixmap_cookie, context_.connection, "graphic::draw_buffer xcb_create_pixmap"))
    {
        return;
    }

    auto image = xcb_image_create_native(context_.connection,
           position.width(), position.height(),
    	   XCB_IMAGE_FORMAT_Z_PIXMAP,
    	   context_.screen->root_depth,
    	   nullptr,
    	   position.width() * position.height() * 4,
		   nullptr);

    if (!image)
    {
    	fprintf(stderr, "WUI error: graphic::draw_buffer xcb_image_create_native\n");
    	return;
    }

    image->data = buffer;

    xcb_image_put(context_.connection, pixmap, gc, image, 0, 0, 0);

    xcb_image_destroy(image);

    auto copy_area_cookie = xcb_copy_area(context_.connection,
        pixmap,
        mem_pixmap,
        gc,
        left_shift,
        top_shift,
        position.left,
        position.top,
        position.right,
        position.bottom);

    xcb_free_pixmap(context_.connection, pixmap);

    if (!check_cookie(copy_area_cookie, context_.connection, "graphic::draw_buffer xcb_copy_area"))
    {
        return;
    }
#endif
}

void graphic::draw_graphic(const rect &position, graphic &graphic_, int32_t left_shift, int32_t top_shift)
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
            top_shift,
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
            top_shift,
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

/// workarounds
void graphic::start_cairo_device()
{
    if (!device)
    {
        device = cairo_device_reference(cairo_surface_get_device(surface));
    }
}

void graphic::end_cairo_device()
{
    if (device)
    {
        cairo_device_finish(device);
        cairo_device_destroy(device);
        device = nullptr;
    }
}

void graphic::draw_surface(_cairo_surface *surface_, const rect &position__)
{
    if (!surface)
    {
        fprintf(stderr, "WUI error no cairo on graphic::draw_surface()");
        return;
    }

    auto cr = cairo_create(surface);

    cairo_set_source_surface(cr, surface_, position__.left, position__.top);
    cairo_paint(cr);

    cairo_destroy(cr);
}

#endif

}
