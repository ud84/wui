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

struct graphic
{
#ifdef _WIN32
    HDC dc;
#elif __linux__
    xcb_connection_t *connection;
    xcb_window_t     wnd;
#endif
};

}
