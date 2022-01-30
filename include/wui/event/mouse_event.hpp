//
// Copyright (c) 2021-2022 Anton Golovkov (udattsk at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/ud84/wui
//

#pragma once

#include <cstdint>

namespace wui
{

enum class mouse_event_type
{
    move,
    enter,
    leave,
    right_down,
    right_up,
    center_down,
    center_up,
    left_down,
    left_up,
    left_double,
    wheel
};

struct mouse_event
{
    mouse_event_type type;

    int32_t x, y;

    int32_t wheel_delta;
};

}
