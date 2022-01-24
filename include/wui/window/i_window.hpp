//
// Copyright (c) 2021-2022 Anton Golovkov (udattsk at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/ud84/wui
//

#pragma once

#include <wui/control/i_control.hpp>
#include <wui/theme/i_theme.hpp>

#include <functional>

#include <string>

namespace wui
{

enum class window_style : uint32_t
{
    resizable = (1 << 0),
    moving = (1 << 1),

    close_button = (1 << 2),
    expand_button = (1 << 3),
    minimize_button = (1 << 4),
    pin_button = (1 << 5),
    title_showed = (1 << 6),

    topmost = (1 << 7),

    above = (1 << 8),
    below = (1 << 9),
    modal = (1 << 10),

    frame = title_showed | close_button | expand_button | minimize_button | resizable | moving,
    dialog = title_showed | close_button | moving,
    pinned = pin_button | close_button | resizable | moving
};

struct system_context;
struct event;
enum class event_type : uint32_t;

class i_window
{
public:
    virtual bool init(const std::string &caption, const rect &position, window_style style, std::function<void(void)> close_callback, std::shared_ptr<i_theme> theme_ = nullptr) = 0;
    virtual void destroy() = 0;

    virtual void add_control(std::shared_ptr<i_control> control, const rect &position) = 0;
    virtual void remove_control(std::shared_ptr<i_control> control) = 0;

    virtual void redraw(const rect &position, bool clear = false) = 0;

    virtual int32_t subscribe(std::function<void(const event&)> receive_callback, event_type event_types, std::shared_ptr<i_control> control = nullptr) = 0;
    virtual void unsubscribe(int32_t subscriber_id) = 0;

    virtual system_context &context() = 0;

protected:
    ~i_window() {}

};

}
