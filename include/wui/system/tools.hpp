//
// Copyright (c) 2021-2022 Anton Golovkov (udattsk at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/ud84/wui
//

#pragma once

#include <wui/system/system_context.hpp>

namespace wui
{

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

#ifdef __linux__
bool check_cookie(xcb_void_cookie_t cookie, xcb_connection_t *connection, const char *err_message);
#endif

}
