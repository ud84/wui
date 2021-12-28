//
// Copyright (c) 2021-2022 Anton Golovkov (udattsk at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/ud84/WUI
//

#pragma once

#include <wui/common/color.hpp>

#include <cstdint>
#include <string>

namespace wui
{

enum class theme
{
    dark,
    white,
    custom
};

enum class theme_value
{
    window_background,
    window_text,
    window_active_button,

    images_path,

    button_calm,
    button_active,
    button_border,
    button_focused_border,
    button_text,
    button_disabled,
    button_round
};

class i_theme
{
public:
    virtual void set_color(theme_value value_id, color color_) = 0;
    virtual color get_color(theme_value value_id) const = 0;

    virtual theme get_theme() const = 0;

    virtual void set_dimension(theme_value value_id, int32_t dimension) = 0;
    virtual int32_t get_dimension(theme_value value_id) const = 0;

    virtual void set_string(theme_value value_id, const std::wstring &value) = 0;
    virtual std::wstring get_string(theme_value value_id) const = 0;

    virtual ~i_theme() {}
};

}
