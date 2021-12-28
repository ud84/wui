//
// Copyright (c) 2021-2022 Anton Golovkov (udattsk at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/ud84/WUI
//

#pragma once

#include <cstdint>

namespace wui
{

enum class keyboard_event_type
{
    down,
    up
};

struct keyboard_event
{
    keyboard_event_type type;

    int32_t key;
};

}
