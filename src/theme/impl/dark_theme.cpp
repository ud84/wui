﻿//
// Copyright (c) 2021-2022 Anton Golovkov (udattsk at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/ud84/WUI
//

#include <wui/theme/impl/dark_theme.hpp>

namespace wui
{

dark_theme::dark_theme()
    : colors
    {
        { theme_value::window_background, make_color(19, 21, 25) },
        { theme_value::window_text, make_color(245, 245, 240) },
        { theme_value::window_active_button, make_color(59, 61, 65) },

        { theme_value::button_calm, make_color(6, 165, 223) },
        { theme_value::button_active, make_color(26, 175, 233) },
        { theme_value::button_border, make_color(0, 160, 210) },
        { theme_value::button_focused_border, make_color(220, 210, 220) },
        { theme_value::button_text, make_color(240, 241, 241) },
        { theme_value::button_disabled, make_color(165, 165, 160) }
    },
    dimensions
    {
        { theme_value::button_round, 5}
    },
    strings
    {
        { theme_value::images_path, L"IMAGES_DARK" }
    }
{
}

void dark_theme::set_color(theme_value value_id, color color_)
{
    colors[value_id] = color_;
}

color dark_theme::get_color(theme_value value_id) const
{
    auto it = colors.find(value_id);
    if (it != colors.end())
    {
        return it->second;
    }
    return 0;
}

theme dark_theme::get_theme() const
{
    return theme::custom;
}

void dark_theme::set_dimension(theme_value value_id, int32_t dimension)
{
    dimensions[value_id] = dimension;
}

int32_t dark_theme::get_dimension(theme_value value_id) const
{
    auto it = dimensions.find(value_id);
    if (it != dimensions.end())
    {
        return it->second;
    }
    return 0;
}

void dark_theme::set_string(theme_value value_id, const std::wstring &value)
{
    strings[value_id] = value;
}

std::wstring dark_theme::get_string(theme_value value_id) const
{
    auto it = strings.find(value_id);
    if (it != strings.end())
    {
        return it->second;
    }
    return L"";
}

}