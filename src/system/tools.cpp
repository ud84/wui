//
// Copyright (c) 2021-2026 Intent Garden Org
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
//

#include <wui/system/tools.hpp>

#include <wui/window/window.hpp>

#include <utf8/utf8.h>

#include <boost/nowide/convert.hpp>

#include <algorithm>

#ifdef _WIN32

#include <windows.h>
#include <tchar.h>

#elif __linux__

#include <pwd.h>

#include <stdlib.h>

#include <xcb/xcb_cursor.h>

#endif

namespace wui
{

    static cursor _cursor{ cursor::no_ }; // предотвращаем затратные операции

    // может понадобится при смене фокуса
    void reset_cursor()
    {
        _cursor = cursor::no_;
    }

#ifdef _WIN32

void set_cursor(system_context &, const cursor cursor_)
{
    if (cursor_ == _cursor)
        return;

    LPCTSTR lcursorName = nullptr;
    switch (cursor_)
    {
        case cursor::default_:
            lcursorName = IDC_ARROW;
        break;
        case cursor::hand:
            lcursorName = IDC_HAND;
        break;
        case cursor::ibeam:
            lcursorName = IDC_IBEAM;
        break;
        case cursor::wait:
            lcursorName = IDC_WAIT;
        break;
        case cursor::size_nwse:
            lcursorName = IDC_SIZENWSE;
        break;
        case cursor::size_nesw:
            lcursorName = IDC_SIZENESW;
        break;
        case cursor::size_we:
            lcursorName = IDC_SIZEWE;
        break;
        case cursor::size_ns:
            lcursorName = IDC_SIZENS;
        break;
    }
    if (lcursorName)
    {
        _cursor = cursor_;
        SetCursor(LoadCursor(NULL, lcursorName));
    }
}

#elif __linux__

void set_cursor(system_context &context, const cursor cursor_)
{
    if (cursor_ == _cursor)
        return;

    const char* cursor_id;

    switch (cursor_)
    {
        case cursor::default_:
            cursor_id = "arrow";
        break;
        case cursor::hand:
            cursor_id = "hand";
        break;
        case cursor::ibeam:
            cursor_id = "xterm";
        break;
        case cursor::wait:
            cursor_id = "wait";
        break;
        case cursor::size_bottom_right:
            cursor_id = "bottom_right_corner";
        break;
        case cursor::size_top_left:
            cursor_id = "top_left_corner";
        break;
        case cursor::size_top_right:
            cursor_id = "top_right_corner";
        break;
        case cursor::size_bottom_left:
            cursor_id = "bottom_left_corner";
        break;
        case cursor::size_we:
            cursor_id = "sb_h_double_arrow";
        break;
        case cursor::size_ns:
            cursor_id = "sb_v_double_arrow";
        break;
    }

    xcb_cursor_context_t *ctx;
    auto screen = xcb_setup_roots_iterator(xcb_get_setup(context.connection)).data;
    if (xcb_cursor_context_new(context.connection, screen, &ctx) >= 0)
    {
        xcb_cursor_t cursor = xcb_cursor_load_cursor(ctx, cursor_id);
        if (cursor != XCB_CURSOR_NONE)
        {
            xcb_change_window_attributes(context.connection, context.wnd, XCB_CW_CURSOR, &cursor);
            _cursor = cursor_;
        }
        else
        {
            cursor_id = "arrow";
            xcb_cursor_t cursor = xcb_cursor_load_cursor(ctx, cursor_id);
            if (cursor != XCB_CURSOR_NONE)
            {
                xcb_change_window_attributes(context.connection, context.wnd, XCB_CW_CURSOR, &cursor);
                _cursor = cursor::default_;
            }
        }
        xcb_cursor_context_free(ctx);
    }
}

bool check_cookie(xcb_void_cookie_t cookie, xcb_connection_t *connection, error &err, std::string_view component)
{
    xcb_generic_error_t *error = xcb_request_check(connection, cookie);
    if (error)
    {
        err.set(error_type::system_error, component, "error code: " + std::to_string(error->error_code));
        return false;
    }
    return true;
}

#endif

void line_up_top_bottom(rect &pos, const int32_t height, const int32_t space)
{
    pos.top = pos.bottom + space;
    pos.bottom = pos.top + height;
}

void line_up_left_right(rect &pos, const int32_t width, const int32_t space)
{
    pos.left = pos.right + space;
    pos.right = pos.left + width;
}

rect get_control_position(const rect& control_position, std::weak_ptr<window> parent)
{
    auto out_pos = control_position;

    auto parent_ = parent.lock();
    if (parent_ && !parent_->parent().expired())
    {
        const rect r = parent_->position();
        out_pos.move(r.left, r.top);
    }

    return out_pos;
}

rect get_popup_position(std::weak_ptr<window> parent, const rect& base_position,
    const rect& popup_control_position, const int32_t indent)
{
    auto parent_ = parent.lock();
    if (!parent_)
    {
        return { };
    }

    auto parent_pos = parent_->position();
    if (parent_->parent().expired()) 
    {
        parent_pos = { 0, 0, parent_pos.width(), parent_pos.height() };
    }

    auto out_pos = popup_control_position;

    bool position_finded = false;

    out_pos.put(base_position.left + indent, base_position.bottom + indent); // below the control
    if (out_pos.bottom <= parent_pos.bottom)
    {
        if (out_pos.right >= parent_pos.right)
        {
            out_pos.put(parent_pos.right - out_pos.width() - indent, base_position.bottom + indent);
        }
        if (out_pos.left < 0)
        {
            out_pos.put(0, base_position.bottom + indent);
        }
        position_finded = true;
    }

    if (!position_finded)
    {
        out_pos.put(base_position.left + indent, base_position.top - out_pos.height() - indent); // above the control
        if (out_pos.top >= parent_pos.top)
        {
            if (out_pos.right >= parent_pos.right)
            {
                out_pos.put(parent_pos.right - out_pos.width(), base_position.top - out_pos.height() - indent);
            }
            //TODO: full test
            const auto ch = parent_->caption_height();
            if (out_pos.top < parent_pos.top + ch)
            {
                out_pos.top = parent_pos.top + ch;
            }
            position_finded = true;
        }
    }

    if (!position_finded)
    {
        out_pos.put(base_position.right + indent, base_position.top + indent); // to the right of the control
        if (out_pos.right <= parent_pos.right)
        {
            position_finded = true;
        }
    }

    if (!position_finded)
    {
        out_pos.put(base_position.left - out_pos.width() - indent, base_position.top + indent); // to the left of the control
        if (out_pos.left >= parent_pos.left)
        {
            position_finded = true;
        }
    }

    if (!position_finded)
    {
        out_pos.put(base_position.left + indent, base_position.top + indent); // on the control
    }

    if (out_pos.bottom > parent_pos.bottom)
    {
        out_pos.move(0, parent_pos.bottom - out_pos.bottom);
        if (out_pos.top < parent_pos.top)
        {
            out_pos.top = parent_pos.top;
            out_pos.bottom = parent_pos.bottom;
        }
        //TODO: full test
        const auto ch = parent_->caption_height();
        if (out_pos.top < parent_pos.top + ch)
        {
            out_pos.top = parent_pos.top + ch;
        }
    }

    if (out_pos.right > parent_pos.right)
    {
        out_pos.move(parent_pos.right - out_pos.right, 0);
        if (out_pos.left < parent_pos.left)
        {
            out_pos.left = parent_pos.left;
            out_pos.right = parent_pos.right;
        }
    }

    parent_pos = parent_->position();
    if (parent_->parent().expired())
    {
        parent_pos = { 0, 0, parent_pos.width(), parent_pos.height() };
    }
    out_pos.move(-parent_pos.left, -parent_pos.top);

    return out_pos;
}

#undef min

namespace
{
    // helper: check "first len bytes - valid UTF-8".
    inline bool prefix_is_valid_utf8(const std::string& s, std::size_t len)
    {
        return utf8::find_invalid(s.begin(), s.begin() + len) == s.begin() + len;
    }

    // helper: nearest valid position <= pos
    std::size_t safe_utf8_cut(const std::string& s, std::size_t pos)
    {
        pos = std::min(pos, s.size());
        while (pos && !prefix_is_valid_utf8(s, pos))
        {
            --pos;  // maximum 3-4 steps (code point length)
        }
        return pos;
    }
}

void truncate_line(std::string& text, graphic* gr, const font& f,
    const int32_t max_width, std::string_view ellipsis)
{
    // 0. Does it fit yet?
    if (measure_text_direct(text, f, gr).width() <= max_width)
    {
        return;
    }

    const int32_t ell_w = measure_text_direct(ellipsis, f, gr).width();
    if (ell_w > max_width)
    {
        text.clear();
        return;
    }

    // 1. Exponential acceleration: find "hi" where width > max_width
    std::size_t lo = 0;
    std::size_t hi = 1;
    const std::size_t n = text.size();

    while (hi < n) {
        const std::size_t cut = safe_utf8_cut(text, hi);
        if (cut == lo)
        {
            hi = std::min(hi * 2, n);
            continue;
        }

        const int32_t w = measure_text(std::string_view(text.data(), cut), f, gr).width() + ell_w;
        if (w > max_width)
            break;

        lo = cut;
        hi = std::min(hi * 2, n);
    }
    hi = std::max<std::size_t>(hi, lo + 1);   // guarantee hi > lo

    // 2. Binary search in [lo, hi)
    while (hi - lo > 1) {
        const std::size_t mid = (lo + hi) / 2;
        const std::size_t cut = safe_utf8_cut(text, mid);
        if (cut == lo)
        {
            hi = lo + 1;
            break; // no progress
        }

        const int32_t w = measure_text(std::string_view(text.data(), cut), f, gr).width() + ell_w;
        (w <= max_width) ? lo = cut : hi = cut;
    }

    // 3. Result
    if (lo < text.size()) {
        text.resize(lo);
        text.append(ellipsis);
    }
}

#ifdef _WIN32
void truncate_line_gdiplus(std::string& text, graphic *gr, const font& f,
    const int32_t max_width, std::string_view ellipsis)
{
    // 0. Does it fit yet?
    if (measure_text_gdiplus_direct(text, f, gr).width() <= max_width)
    {
        return;
    }

    const int32_t ell_w = measure_text_gdiplus_direct(ellipsis, f, gr).width();
    if (ell_w > max_width)
    {
        text.clear();
        return;
    }

    // 1. Exponential acceleration: find "hi" where width > max_width
    std::size_t lo = 0;
    std::size_t hi = 1;
    const std::size_t n = text.size();

    while (hi < n) {
        const std::size_t cut = safe_utf8_cut(text, hi);
        if (cut == lo)
        {
            hi = std::min(hi * 2, n);
            continue;
        }

        const int32_t w = measure_text_gdiplus(std::string_view(text.data(), cut), f, gr).width() + ell_w;
        if (w > max_width)
            break;

        lo = cut;
        hi = std::min(hi * 2, n);
    }
    hi = std::max<std::size_t>(hi, lo + 1);   // guarantee hi > lo

    // 2. Binary search in [lo, hi)
    while (hi - lo > 1) {
        const std::size_t mid = (lo + hi) / 2;
        const std::size_t cut = safe_utf8_cut(text, mid);
        if (cut == lo)
        {
            hi = lo + 1;
            break; // no progress
        }

        const int32_t w = measure_text_gdiplus(std::string_view(text.data(), cut), f, gr).width() + ell_w;
        (w <= max_width) ? lo = cut : hi = cut;
    }

    // 3. Result
    if (lo < text.size()) {
        text.resize(lo);
        text.append(ellipsis);
    }
}
#endif

}
