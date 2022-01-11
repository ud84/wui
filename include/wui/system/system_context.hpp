//
// Copyright (c) 2021-2022 Anton Golovkov (udattsk at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/ud84/wui
//

#pragma once

#ifdef _WIN32
#include <windows.h>
#elif __linux__
#include <xcb/xcb.h>
#endif

namespace wui
{

#ifdef _WIN32

struct system_context
{
    HWND hwnd;
    HDC dc;
};

#elif __linux__

struct system_context
{
    xcb_connection_t *connection;
    xcb_screen_t     *screen;
    xcb_window_t     wnd;
};

#endif

}
