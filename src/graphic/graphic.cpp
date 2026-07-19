//
// Copyright (c) 2021-2026 Intent Garden Org
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
//

#include <wui/graphic/graphic.hpp>
#include <wui/system/tools.hpp>
#include <wui/common/error.hpp>

#include <boost/nowide/convert.hpp>

#include <unordered_map>

#ifdef __linux__
#include <xcb/xcb_image.h>

#include <cairo.h>
#include <cairo-xcb.h>

#include <algorithm>
#include <cmath>
#include <memory>
#elif _WIN32
#include <gdiplus.h>
#endif

namespace wui
{

#ifdef __linux__
static xcb_visualtype_t *default_visual_type(wui::system_context &context_)
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
#endif

graphic::graphic(system_context &context__)
    : context_(context__),
      pc(context_),
      max_size_(),
      background_color(make_color(0, 0, 0, 0))
#ifdef _WIN32
    , mem_dc(NULL),
      mem_bitmap(NULL)
#elif __linux__
    , mem_pixmap(0),
      surface(nullptr),
      device(nullptr)
#endif
{
}

graphic::~graphic()
{
    release();
}

bool graphic::inited() const noexcept
{
#ifdef _WIN32
    return NULL != mem_bitmap;
#elif __linux__
    return nullptr != surface;
#endif
}

bool graphic::init(const rect &max_size__, const color background_color_)
{
    if (inited())
    {
        err.set(error_type::already_started, "graphic::init()", "the object 'graphic' are already initialized");
        return false;
    }

    max_size_ = max_size__;
    background_color = background_color_;

#ifdef _WIN32


    auto wnd_dc = GetDC(context_.hwnd);

    mem_dc = CreateCompatibleDC(wnd_dc);

    mem_bitmap = CreateCompatibleBitmap(wnd_dc, max_size_.width(), max_size_.height());
    if (!mem_bitmap)
    {
        ReleaseDC(context_.hwnd, wnd_dc);
        err.set(error_type::no_handle, "graphic::init()", "CreateCompatibleBitmap returns null");
        return false;
    }

    SelectObject(mem_dc, mem_bitmap);

    SetMapMode(mem_dc, MM_TEXT);

    RECT filling_rect = { 0, 0, max_size_.width(), max_size_.height() };
    FillRect(mem_dc, &filling_rect, pc.get_brush(background_color));

    ReleaseDC(context_.hwnd, wnd_dc);

    //err.reset(); // сохраняем ошибки, для публикации

#elif __linux__

    if (!context_.connection)
    {
        err.set(error_type::no_handle, "graphic::init()");
        return false;
    }

    mem_pixmap = xcb_generate_id(context_.connection);
    auto pixmap_create_cookie = xcb_create_pixmap(context_.connection,
        context_.screen->root_depth,
        mem_pixmap,
        context_.wnd,
        max_size_.width(),
        max_size_.height());
    if (!check_cookie(pixmap_create_cookie, context_.connection, err, "graphic::init() xcb_create_pixmap."))
    {
        mem_pixmap = 0;
        err.set(error_type::no_handle, "graphic::init() xcb_create_pixmap",
            "Can't create the pixmap");
        return false;
    }

    surface = cairo_xcb_surface_create(context_.connection, mem_pixmap,
        default_visual_type(context_), max_size_.width(), max_size_.height());
    if (!surface)
    {
        xcb_free_pixmap(context_.connection, mem_pixmap);
        mem_pixmap = 0;

        err.set(error_type::no_handle, "graphic::init() cairo_xcb_surface_create",
            "Can't create the cairo surface");
        return false;
    }

    clear(max_size_);

    //err.reset();  // сохраняем ошибки, для публикации
#endif

    pc.init();
    return true;
}

void graphic::release()
{
    pc.release();

    if (!inited())
        return;

#ifdef _WIN32
    DeleteObject(mem_bitmap);
    mem_bitmap = NULL;

    DeleteDC(mem_dc);
    mem_dc = NULL;
#elif __linux__

    cairo_surface_destroy(surface);
    surface = nullptr;

    if (context_.connection && mem_pixmap)
    {
        auto free_pixmap_cookie = xcb_free_pixmap(context_.connection, mem_pixmap);
        check_cookie(free_pixmap_cookie, context_.connection, err, "graphic::release()");
        mem_pixmap = 0;
    }
#endif
}

rect graphic::max_size() const noexcept
{
    return max_size_;
}

void graphic::set_background_color(color background_color_)
{
    background_color = background_color_;

    clear({ 0, 0, max_size_.width(), max_size_.height() });
}

void graphic::clear(const rect& position)
{
    if (!inited())
    {
        return;
    }

#ifdef _WIN32

    RECT filling_rect = !position.is_null() ?
        RECT{ position.left, position.top, position.right, position.bottom } :
        RECT{ 0, 0, max_size_.right, max_size_.bottom };
    FillRect(mem_dc, &filling_rect, pc.get_brush(background_color));
#elif __linux__
    auto cr = cairo_create(surface);

    cairo_set_source_rgb(cr, static_cast<double>(wui::get_red(background_color)) / 255,
        static_cast<double>(wui::get_green(background_color)) / 255,
        static_cast<double>(wui::get_blue(background_color)) / 255);
    if (!position.is_null())
        cairo_rectangle(cr, position.left, position.top, position.width(), position.height());
    else
        cairo_rectangle(cr, 0, 0, max_size_.right, max_size_.bottom);
    cairo_fill(cr);

    cairo_destroy(cr);
#endif
}

void graphic::flush(const rect& updated_size)
{
    if (!inited())
        return;

#ifdef _WIN32
    auto wnd_dc = GetDC(context_.hwnd);

    if (wnd_dc)
    {
        BitBlt(wnd_dc,
            updated_size.left,
            updated_size.top,
            updated_size.width(),
            updated_size.height(),
            mem_dc,
            updated_size.left,
            updated_size.top,
            SRCCOPY);
        ReleaseDC(context_.hwnd, wnd_dc);
    }

#elif __linux__
    if (context_.wnd)
    {
        auto copy_area_cookie = xcb_copy_area(context_.connection,
            mem_pixmap,
            context_.wnd,
            pc.get_gc(background_color),
            updated_size.left,
            updated_size.top,
            updated_size.left,
            updated_size.top,
            updated_size.width(),
            updated_size.height());

        if (!check_cookie(copy_area_cookie, context_.connection, err, "graphic::end_drawing xcb_copy_area"))
        {
            return;
        }
    }
#endif
}

void graphic::draw_pixel(const rect& position, const color color_)
{
#ifdef _WIN32
    // color alpha not supported
    SetPixel(mem_dc, position.left, position.top, color_); // color_ & 0x00FFFFFF
#elif __linux__
    xcb_point_t points[] = { { static_cast<int16_t>(position.left), static_cast<int16_t>(position.top) } };
    xcb_poly_point(context_.connection, XCB_COORD_MODE_ORIGIN, mem_pixmap,
        pc.get_gc(color_),
        1, points);
#endif
}

// NB: linux width = 1 always
void graphic::draw_line(const rect& position, const color color_, const int32_t width)
{
#ifdef _WIN32
    auto old_pen = (HPEN)SelectObject(mem_dc, pc.get_pen(PS_SOLID, width, color_));

    MoveToEx(mem_dc, position.left, position.top, (LPPOINT)NULL);
    LineTo(mem_dc, position.right, position.bottom);

    SelectObject(mem_dc, old_pen);
#elif __linux__
    xcb_point_t polyline[] = { { static_cast<int16_t>(position.left), static_cast<int16_t>(position.top) },
        { static_cast<int16_t>(position.right), static_cast<int16_t>(position.bottom) } };
    xcb_poly_line(context_.connection, XCB_COORD_MODE_ORIGIN, mem_pixmap,
        pc.get_gc(color_),
        2, polyline);
#endif
}

rect graphic::measure_text(std::string_view text_, const font &font__)
{
    if (text_.empty() || !inited())
    {
        return {0, 0, 0, font__.size};
    }

#ifdef _WIN32
    auto old_font = (HFONT)SelectObject(mem_dc, pc.get_font(font__));

    SIZE sz{ };
    auto wide_str = boost::nowide::widen(text_);
    GetTextExtentPoint32W(mem_dc, wide_str.c_str(), static_cast<int>(wide_str.size()), &sz);

    //RECT text_rect{ };
    // DrawText приводит к отказам GDI вне контекста сообщений WM_PAINT (WM_PRINTCLIENT ?)
    // .. то есть вне методов draw()
    //DrawTextW(mem_dc, wide_str.c_str(), static_cast<int>(wide_str.size()), &text_rect, DT_NOPREFIX|DT_CALCRECT);

    SelectObject(mem_dc, old_font);

    return {0, 0, sz.cx, sz.cy };
#elif __linux__

    auto cr = pc.get_font(font__, surface);
    if (!cr)
    {
        err.set(error_type::no_handle, "graphic::measure_text()", "No cairo font context");
        return rect{ };
    }

    cairo_text_extents_t dot_extents, extents;   // It's a workaround 'magic'
    cairo_text_extents(cr, ".", &dot_extents);   // to work the spaces
    if (CAIRO_STATUS_SUCCESS != cairo_status(cr))
    {
        return { 0, 0, 0, font__.size };
    }

    std::string s; s.reserve(text_.size() + 2);  //
    s = '.' + std::string(text_) + '.';          // =)
    cairo_text_extents(cr, s.c_str(), &extents);
    if(CAIRO_STATUS_SUCCESS != cairo_status(cr))
    {
        return { 0, 0, 0, font__.size };
    }

    return { 0, 0,
        static_cast<int32_t>(ceil(extents.width - (dot_extents.width * 3))),
        static_cast<int32_t>(ceil(extents.height)) };
#endif
}

#ifdef _WIN32

rect graphic::measure_text_gdiplus(std::string_view text_, const font &font__)
{
    if (text_.empty() || !inited())
    {
        return {0, 0, 0, font__.size};
    }
    auto old_font = (HFONT)SelectObject(mem_dc, pc.get_font(font__));

    Gdiplus::Font font(mem_dc);

    //RECT client_rect;
    //GetClientRect(context_.hwnd, &client_rect);
    Gdiplus::RectF layoutRect{ };
    //(0, 0, static_cast<Gdiplus::REAL>(client_rect.right - client_rect.left),
    //static_cast<Gdiplus::REAL>(client_rect.bottom - client_rect.top));

    auto wide_str = boost::nowide::widen(text_);
    Gdiplus::RectF boundingBox;
    Gdiplus::Graphics g(mem_dc);

    //const Gdiplus::StringFormat stringFormat(Gdiplus::StringFormat::GenericTypographic());
    // - убирает добавку в начале и в конце строки, но "..." ставит не точно (пример simple)

    const Gdiplus::Status status = g.MeasureString(
        wide_str.c_str(),
        static_cast<INT>(wide_str.size()),
        &font,
        layoutRect,
        //&stringFormat,
        &boundingBox
    );

    SelectObject(mem_dc, old_font);
    if (Gdiplus::Ok == status)
    {
        return { 0, 0, static_cast<int32_t>(boundingBox.GetRight()),
            static_cast<int32_t>(boundingBox.GetBottom()) };
    }
    return { 0, 0, 0, font__.size };
}

#endif

void graphic::draw_text(const rect &position, std::string_view text_, const color color_, const font &font__)
{
#ifdef _WIN32
    auto old_font = (HFONT)SelectObject(mem_dc, pc.get_font(font__));

    SetTextColor(mem_dc, get_rgb(color_));
    SetBkMode(mem_dc, TRANSPARENT);

    auto wide_str = boost::nowide::widen(text_);
#if 1
    TextOutW(mem_dc, position.left, position.top, wide_str.c_str(), static_cast<int>(wide_str.size()));
#else
    ExtTextOutW(mem_dc,
        position.left, position.top,
        ETO_CLIPPED,
        NULL, wide_str.c_str(),
        static_cast<UINT>(wide_str.size()), NULL);
#endif
    SelectObject(mem_dc, old_font);
#elif __linux__

    auto cr = pc.get_font(font__, surface);
    if (!cr)
    {
        err.set(error_type::no_handle, "graphic::draw_text()", "No cairo font context");
        return;
    }

    cairo_set_source_rgb(cr,
        static_cast<double>(wui::get_red(color_)) / 255,
        static_cast<double>(wui::get_green(color_)) / 255,
        static_cast<double>(wui::get_blue(color_)) / 255
        //, static_cast<double>(wui::get_alpha(color_)) / 255 // rgba
    );

    cairo_move_to(cr, position.left, position.top + font__.size * 5.0 / 6.0);

    std::string text__(text_); /// Workaround to prevent crashes
    text__ += '\0';

    cairo_show_text(cr, text__.c_str());
#endif
}

#ifdef _WIN32

void graphic::draw_text_clip_rgb(const rect& position, const text_lines_t& lines,
    const color color_, const font& font__, const bool clip_)
{
    auto old_font = (HFONT)SelectObject(mem_dc, pc.get_font(font__));
    SetTextColor(mem_dc, get_rgb(color_));
    SetBkMode(mem_dc, TRANSPARENT);

    const RECT rc = { position.left, position.top,
        position.right, position.bottom };
    const RECT* ptr_rc = clip_ ? &rc : nullptr;
    for (auto& line : lines)
    {
        auto wide_str = boost::nowide::widen(line.str);
        ExtTextOutW(mem_dc,
            position.left + line.rc.left, position.top + line.rc.top,
            ETO_CLIPPED,
            ptr_rc, wide_str.c_str(),
            static_cast<UINT>(wide_str.size()), NULL);
    }
    SelectObject(mem_dc, old_font);
}

// supported alpha
void graphic::draw_text_clip(const rect& position, const text_lines_t& lines,
    const color color_, const font& font__, const bool clip_)
{
    auto old_font = (HFONT)SelectObject(mem_dc, pc.get_font(font__));
    Gdiplus::Graphics g(mem_dc);
    if (clip_)
    {
        Gdiplus::Region region(Gdiplus::Rect(position.left, position.top, position.width(), position.height()));
        g.SetClip(&region, Gdiplus::CombineModeReplace);
    }

    const Gdiplus::Font font(mem_dc);
    const Gdiplus::SolidBrush br
    (
        Gdiplus::Color(get_alpha(color_), get_red(color_), get_green(color_), get_blue(color_))
    );

    //Gdiplus::StringFormat stringFormat(Gdiplus::StringFormat::GenericTypographic());
    // - убирает добавку в начале и в конце строки, но "..." ставит не точно (пример simple)

    for (auto& line : lines)
    {
        auto wide_str = boost::nowide::widen(line.str);
        const Gdiplus::PointF pt(static_cast<Gdiplus::REAL>(position.left + line.rc.left),
            static_cast<Gdiplus::REAL>(position.top + line.rc.top));
        g.DrawString(wide_str.c_str(),
            static_cast<INT>(wide_str.size()),
            &font,
            pt,
            //&stringFormat,
            &br);
    }

    SelectObject(mem_dc, old_font);
}

#elif __linux__

// supported alpha
void graphic::draw_text_clip(const rect & position, const text_lines_t & lines,
    const color color_, const font & font__, const bool clip_)
{
    auto cr = pc.get_font(font__, surface);
    if (!cr) {
        err.set(error_type::no_handle, "graphic::draw_text()", "No cairo font context");
        return;
    }

    cairo_set_source_rgba(cr,
        static_cast<double>(wui::get_red(color_)) / 255,
        static_cast<double>(wui::get_green(color_)) / 255,
        static_cast<double>(wui::get_blue(color_)) / 255,
        static_cast<double>(wui::get_alpha(color_)) / 255);

    if (clip_)
    {
        cairo_rectangle(cr, position.left, position.top, position.width(), position.height());
        cairo_clip(cr);
    }

    std::string text;
    for (auto& line : lines)
    {
        cairo_move_to(cr, position.left + line.rc.left, position.top + line.rc.top + font__.size * 5.0 / 6.0);
        text = line.str; /// Workaround to prevent crashes
        text += '\0';

        cairo_show_text(cr, text.c_str());
    }

    if (clip_)
    {
        cairo_reset_clip(cr);
    }
}
#endif

void graphic::draw_rect(const rect& position, const color fill_color)
{
#ifdef _WIN32
    RECT position_rect = { position.left, position.top, position.right, position.bottom };
    FillRect(mem_dc, &position_rect, pc.get_brush(fill_color));
#elif __linux__
    //assert(surface);
    //if (!surface)
    //{
    //    err.set(error_type::no_handle, "graphic::draw_rect()", "No cairo surface");
    //    return;
    //}

    auto pos = position;
    if (pos.left > pos.right)
    {
        std::swap(pos.left, pos.right);
    }
    if (pos.top > pos.bottom)
    {
        std::swap(pos.top, pos.bottom);
    }

    auto cr = cairo_create(surface);

    cairo_set_source_rgb(cr, static_cast<double>(get_red(fill_color)) / 255,
        static_cast<double>(get_green(fill_color)) / 255,
        static_cast<double>(get_blue(fill_color)) / 255);
    cairo_rectangle(cr, pos.left, pos.top, pos.width(), pos.height());
    cairo_fill(cr);

    cairo_destroy(cr);
#endif
}

#ifdef _WIN32
static void DrawRoundBox(HDC dc, const rect &pos_, const int32_t radius_,
    const int32_t borderWidth, const color background, const color border)
{
    Gdiplus::Graphics g(dc);

    // Make the path
    const int32_t shift1 = (borderWidth > 0 ? -borderWidth : borderWidth)/2;
    const int32_t shift2 = static_cast<int32_t>(round((borderWidth > 0 ? borderWidth : -borderWidth)/2.0));
    const rect pos{ pos_.left - shift1, pos_.top - shift1,
        pos_.right - shift2, pos_.bottom - shift2 };

    Gdiplus::GraphicsPath path;
    if (radius_)
    {
        g.SetSmoothingMode(Gdiplus::SmoothingModeAntiAlias);
        const int radius = radius_ > 0 ? radius_ : -radius_;
        path.AddArc(pos.left, pos.top, radius * 2, radius * 2, 180, 90);
        path.AddArc(pos.right - radius * 2, pos.top, radius * 2, radius * 2, 270, 90);
        path.AddArc(pos.right - radius * 2, pos.bottom - radius * 2, radius * 2, radius * 2, 0, 90);
        path.AddArc(pos.left, pos.bottom - radius * 2, radius * 2, radius * 2, 90, 90);
        path.CloseFigure();
    }
    else
    {
        Gdiplus::Point p[] = {
            { pos.left,  pos.top },
            { pos.right, pos.top },
            { pos.right, pos.bottom},
            { pos.left,  pos.bottom}
        };
        path.AddLines(p, 4);
        path.CloseFigure();
    }

    // Fill (only if the color is set)
    if (0 != get_alpha(background))
    {
        Gdiplus::Color fill(
            get_alpha(background), // 255, opaque
            get_red(background),
            get_green(background),
            get_blue(background)
        );

        Gdiplus::SolidBrush br(fill);
        g.FillPath(&br, &path);
    }

    // Border outline
    if (borderWidth && 0 != get_alpha(border))
    {
        Gdiplus::Pen pen(
            Gdiplus::Color(
                get_alpha(border),
                get_red(border),
                get_green(border),
                get_blue(border)
            ),
            1.0f * (borderWidth > 0 ? borderWidth : -borderWidth));
        g.DrawPath(&pen, &path);
    }
}
#endif

void graphic::draw_rect(const rect& position, const color border_color,
    const color fill_color, const uint32_t border_width, const uint32_t rnd)
{
#ifdef _WIN32
    DrawRoundBox(mem_dc, position, rnd, border_width, fill_color, border_color);
#elif __linux__

    auto cr = cairo_create(surface);

    const double l = position.left,
       t     = position.top,
       r     = position.right,
       b     = position.bottom,
       width  = position.width(),
       height = position.height();

    cairo_new_sub_path(cr);

    if (0 == rnd)
    {
        cairo_rectangle(cr, l, t, width, height);
    }
    else
    {
        const double radius = rnd > 0 ? rnd : -rnd;
        constexpr double degrees = M_PI / 180.0;

        cairo_arc (cr, l + width - radius, t + radius, radius, -90 * degrees, 0 * degrees);
        cairo_arc (cr, l + width - radius, t + height - radius, radius, 0 * degrees, 90 * degrees);
        cairo_arc (cr, l + radius, t + height - radius, radius, 90 * degrees, 180 * degrees);
        cairo_arc (cr, l + radius, t + radius, radius, 180 * degrees, 270 * degrees);
    }

    cairo_close_path(cr);

    if (0 != get_alpha(fill_color))
    {
        cairo_set_source_rgba(cr,
            static_cast<double>(get_red(fill_color)) / 255,
            static_cast<double>(get_green(fill_color)) / 255,
            static_cast<double>(get_blue(fill_color)) / 255
            , static_cast<double>(get_alpha(fill_color)) / 255
        );
        cairo_fill_preserve(cr);
    }

    if (border_width && 0 != get_alpha(border_color))
    {
        cairo_set_source_rgba(cr,
            static_cast<double>(get_red(border_color)) / 255,
            static_cast<double>(get_green(border_color)) / 255,
            static_cast<double>(get_blue(border_color)) / 255
            , static_cast<double>(get_alpha(border_color)) / 255
        );
        cairo_set_line_width(cr, border_width > 0 ? border_width : -border_width);
        cairo_stroke(cr);
    }

    cairo_destroy(cr);

#endif
}

void graphic::draw_buffer(const rect& position,
    uint8_t *buffer, const int32_t left_shift, const int32_t top_shift)
{

#ifdef _WIN32
    if (!mem_dc)
        return;

    auto source_bitmap = pc.get_bitmap(position.width(), position.height(), buffer, mem_dc);
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

    DeleteDC(source_dc);
#elif __linux__

    if (!context_.connection)
        return;

    auto pixmap = xcb_generate_id(context_.connection);
    auto pixmap_cookie = xcb_create_pixmap(context_.connection,
        context_.screen->root_depth,
        pixmap,
        context_.wnd,
        position.width(), position.height());

    if (!check_cookie(pixmap_cookie, context_.connection, err, "graphic::draw_buffer() xcb_create_pixmap"))
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
        xcb_free_pixmap(context_.connection, pixmap);

        err.set(error_type::no_handle, "graphic::draw_buffer()", "xcb_image_create_native error");
        return;
    }

    image->data = buffer;

    xcb_image_put(context_.connection, pixmap, pc.get_gc(background_color), image, 0, 0, 0);

    xcb_image_destroy(image);

    auto copy_area_cookie = xcb_copy_area(context_.connection,
        pixmap,
        mem_pixmap,
        pc.get_gc(background_color),
        left_shift,
        top_shift,
        position.left,
        position.top,
        position.width(),
        position.height()
        //position.right, // ?
        //position.bottom // ?
    );

    xcb_free_pixmap(context_.connection, pixmap);

    if (!check_cookie(copy_area_cookie, context_.connection, err, "graphic::draw_buffer() xcb_copy_area"))
    {
        return;
    }
#endif
}

void graphic::draw_graphic(const rect& position, graphic &graphic_,
    const int32_t left_shift, const int32_t top_shift)
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
            pc.get_gc(graphic_.background_color), //old: this->background_color
            left_shift,
            top_shift,
            position.left,
            position.top,
            position.right,
            position.bottom);

        if (!check_cookie(copy_area_cookie, context_.connection, err, "graphic::draw_graphic() xcb_copy_area"))
        {
            return;
        }
    }
#endif
}

#ifdef _WIN32
#elif __linux__

/// workarounds
void graphic::draw_surface(cairo_surface_t &surface_, const rect& position__)
{
    auto cr = cairo_create(surface);

    auto surface_width = cairo_image_surface_get_width(&surface_);
    auto surface_height = cairo_image_surface_get_height(&surface_);

    double x_scale_factor = 1.0, y_scale_factor = 1.0;

    if (surface_width != 0 && surface_height != 0)
    {
        x_scale_factor = static_cast<double>(position__.width()) / surface_width;
        y_scale_factor = static_cast<double>(position__.height()) / surface_height;

        cairo_scale(cr, x_scale_factor, y_scale_factor);
    }

    cairo_set_source_surface(cr,
        &surface_,
        position__.left / x_scale_factor,
        position__.top / y_scale_factor);

    cairo_paint(cr);

    cairo_destroy(cr);
}

#endif

error graphic::get_error() const
{
    return err;
}

/// Text measurer //////////////////////////

static graphic* tm_graphic = nullptr;

void graphic::set_text_measurer(graphic* gr) noexcept
{
    tm_graphic = gr;
}

//NB: ? добавить для совместимости с wui-1.3.260215
//void init_text_measurer(graphic* gr) noexcept
//{
//    tm_graphic = gr;
//}

graphic* graphic::get_text_measurer() noexcept
{
    return tm_graphic;
}

bool graphic::text_measurer_inited() noexcept
{
    return nullptr != tm_graphic;
}
bool graphic::is_me_text_measurer(const graphic& gr) noexcept
{
#ifdef _WIN32
    return tm_graphic && gr.context_.hwnd == tm_graphic->context_.hwnd;
#elif __linux__
    return tm_graphic && gr.context_.wnd == tm_graphic->context_.wnd;
#endif
}


static std::unordered_map<std::string, std::pair<int32_t, int32_t>> tm_cache;

static size_t font_hash(const font &font_)
{
    std::hash<std::string> hasher;

    return hasher(font_.name + std::to_string(font_.size));
}

rect measure_text(std::string_view text, const font& font_, graphic* gr)
{
    if (text.empty())
    {
        return { 0, 0, 0, font_.size };
    }

    std::hash<std::string> hasher;
    // Compute the hash value
    auto stringHash = hasher(std::string(text));
    auto fontHash = font_hash(font_);
    auto hash = std::to_string(stringHash) + "_" + std::to_string(fontHash);

    // Find in cache if finded - return
    auto it = tm_cache.find(hash);
    if (it != tm_cache.end())
    {
        return { 0, 0, it->second.first, it->second.second };
    }

    gr = (gr && gr->inited()) ? gr : (tm_graphic && tm_graphic->inited()) ? tm_graphic : nullptr;
    if (!gr)
    {
        return { 0, 0, 0, font_.size };
    }

    // Measure and cache
    const auto pos = gr->measure_text(text, font_);
    tm_cache[hash] = { pos.width(), pos.height() };
    return pos;
}

rect measure_text_direct(std::string_view text, const font& font_, graphic* gr)
{
    if (text.empty())
    {
        return { 0, 0, 0, font_.size };
    }
    gr = (gr && gr->inited()) ? gr : (tm_graphic && tm_graphic->inited()) ? tm_graphic : nullptr;
    if (!gr)
    {
        return { 0, 0, 0, font_.size };
    }
    return gr->measure_text(text, font_);
}

#ifdef _WIN32

//NB: width, height gdi+ отличаются от сохраненных в tm_cache
static std::unordered_map<std::string, std::pair<int32_t, int32_t>> tm_cache_gdi;

rect measure_text_gdiplus(std::string_view text, const font& font_, graphic* gr)
{
    if (text.empty())
    {
        return { 0, 0, 0, font_.size };
    }

    std::hash<std::string> hasher;

    // Compute the hash value
    auto stringHash = hasher(std::string(text));
    auto fontHash = font_hash(font_);
    auto hash = std::to_string(stringHash) + "_" + std::to_string(fontHash);

    // Find in cache if finded - return
    auto it = tm_cache_gdi.find(hash);
    if (it != tm_cache_gdi.end())
    {
        return { 0, 0, it->second.first, it->second.second };
    }

    gr = (gr && gr->inited()) ? gr : (tm_graphic && tm_graphic->inited()) ? tm_graphic : nullptr;
    if (!gr)
    {
        return { 0, 0, 0, font_.size };
    }

    // Measure and cache
    auto pos = gr->measure_text_gdiplus(text, font_);
    tm_cache_gdi[hash] = { pos.width(), pos.height() };
    return pos;
}

rect measure_text_gdiplus_direct(std::string_view text, const font& font_, graphic* gr)
{
    if (text.empty())
    {
        return { 0, 0, 0, font_.size };
    }

    gr = (gr && gr->inited()) ? gr : (tm_graphic && tm_graphic->inited()) ? tm_graphic : nullptr;
    if (!gr)
    {
        return { 0, 0, 0, font_.size };
    }
    return gr->measure_text_gdiplus(text, font_);
}
#endif

}
