//
// Copyright (c) 2021-2022 Anton Golovkov (udattsk at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/ud84/wui
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
    default_,
    hand,
    ibeam,
    wait,
    size_nwse,
    size_nesw,
    size_we,
    size_ns
};

void set_cursor(system_context &context, cursor cursor_);

/// This function should be used when changing the position of a control (inside the set_position() of the control)
void update_control_position(rect &control_position,
    const rect &new_control_position,
    bool redraw,
    std::weak_ptr<window> parent);

/// This function helps to place controls on the window from top to bottom
void line_up_top_bottom(rect &pos, int32_t height, int32_t space);

/// This function helps to place controls on the window from left to right
void line_up_left_right(rect &pos, int32_t width, int32_t space);

/// This function returns the absolute position of the control on the physical window. Must be called inside the control's position() method
rect get_control_position(const rect &control_position, std::weak_ptr<window> parent);

/// This function calculates the position of the popup item relative to base position
rect get_popup_position(std::weak_ptr<window> parent, const rect &base_position, const rect &popup_control_position, int32_t indent);

/// This function truncates the string
void truncate_line(std::string &line, graphic &gr, const font &font_, int32_t width, int32_t truncating_count = 10);

/// Service on Linux
#ifdef __linux__
bool check_cookie(xcb_void_cookie_t cookie, xcb_connection_t *connection, error &err, std::string_view component);
#endif

}
