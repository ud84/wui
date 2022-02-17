//
// Copyright (c) 2021-2022 Anton Golovkov (udattsk at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/ud84/wui
//

#pragma once

namespace wui
{

enum class internal_event_type
{
    execute_focused,
    
    size_changed,
    position_changed,
    
    pin_clicked,

    user_emitted
};

struct internal_event
{
    internal_event_type type;
    int32_t x, y;
};

}
