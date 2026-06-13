//
// Copyright (c) 2021-2026 Intent Garden Org
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
//

#pragma once

#include <memory>
#include <string>

#include <wui/common/rect.hpp>
#include <wui/common/font.hpp>
#include <wui/common/error.hpp>
#include <wui/system/system_context.hpp>

namespace wui
{

class window;

enum class cursor
{
    no_,
    default_,
    hand,
    ibeam,
    wait,
#ifdef _WIN32
    size_nwse,
    size_nesw,
#elif __linux__
    size_bottom_right,
    size_top_left,
    size_top_right,
    size_bottom_left,
#endif
    size_we,
    size_ns
};

void set_cursor(system_context &context, const cursor cursor_);
void reset_cursor();

/// This function helps to place controls on the window from top to bottom
void line_up_top_bottom(rect &pos, const int32_t height, const int32_t space);

/// This function helps to place controls on the window from left to right
void line_up_left_right(rect &pos, const int32_t width, const int32_t space);

/// This function returns the absolute position of the control on the physical window. Must be called inside the control's position() method
rect get_control_position(const rect &control_position, std::weak_ptr<window> parent);

/// This function calculates the position of the popup item relative to base position
rect get_popup_position(std::weak_ptr<window> parent, const rect& base_position,
    const rect& popup_control_position, const int32_t indent);

/// This function truncates the string
void truncate_line(std::string &line, graphic *gr, const font &font_,
    const int32_t width, std::string_view ellipsis = "…");
#ifdef _WIN32
void truncate_line_gdiplus(std::string &line, graphic *gr, const font &font_,
    const int32_t width, std::string_view ellipsis = "…");
#endif
/// Service on Linux
#ifdef __linux__
bool check_cookie(xcb_void_cookie_t cookie, xcb_connection_t *connection, error &err, std::string_view component);
#endif

}
