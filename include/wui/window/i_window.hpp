//
// Copyright (c) 2021-2022 Anton Golovkov (udattsk at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/ud84/WUI
//

#pragma once

#include <wui/control/i_control.hpp>
#include <wui/theme/i_theme.hpp>

#include <functional>

#include <string>

namespace wui
{

enum class window_type
{
    frame,
    dialog
};

class i_window
{
public:
    virtual bool init(window_type type, const rect &position, const std::wstring &caption, std::function<void(void)> close_callback, std::shared_ptr<i_theme> theme_ = nullptr) = 0;
    virtual void destroy() = 0;

    virtual void add_control(std::shared_ptr<i_control> control, const rect &position) = 0;
    virtual void remove_control(std::shared_ptr<i_control> control) = 0;

    virtual void redraw(const rect &position, bool clear = false) = 0;

protected:
    ~i_window() {}

};

}
