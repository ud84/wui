//
// Copyright (c) 2021-2026 Intent Garden Org
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
//

#pragma once

#include <wui/event/mouse_event.hpp>
#include <wui/event/keyboard_event.hpp>
#include <wui/event/internal_event.hpp>
#include <wui/event/system_event.hpp>

namespace wui
{

enum class event_type : uint32_t
{
    system = (1 << 0),
    mouse = (1 << 1),
    keyboard = (1 << 2),
    internal = (1 << 3),

    all = system | mouse | keyboard | internal
};

// event_type b = wui::event_type::keyboard | wui::event_type::system;
inline constexpr event_type operator|(const event_type l, const event_type r)
{
    return static_cast <event_type> (static_cast <uint32_t> (l) | static_cast <uint32_t> (r));
}

// bool b = keyboard & internal;
inline constexpr bool operator&(const event_type l, const event_type r)
{
    return 0 != (static_cast <uint32_t> (l) & static_cast <uint32_t> (r));
}
/*

inline event_type test1(const event_type e1, const event_type e2, const event_type e3)
{
    return e1 | e2 | e3;
}
inline bool test2(const event_type e1, const event_type e2)
{
    return (e1 & e2);
}
*/

struct event
{
    event_type type;

    union
    {
        mouse_event mouse_event_;
        keyboard_event keyboard_event_;
        internal_event internal_event_;
        system_event system_event_;
    };
};

}
