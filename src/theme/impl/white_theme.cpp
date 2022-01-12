//
// Copyright (c) 2021-2022 Anton Golovkov (udattsk at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/ud84/WUI
//

#include <wui/theme/impl/white_theme.hpp>

namespace wui
{

white_theme::white_theme()
    : colors
    {
        { theme_value::window_background, make_color(240, 240, 240) },
        { theme_value::window_text, make_color(25, 25, 20) },
        { theme_value::window_active_button, make_color(220, 220, 220) },

        { theme_value::button_calm, make_color(6, 165, 223) },
        { theme_value::button_active, make_color(26, 175, 233) },
        { theme_value::button_border, make_color(0, 160, 210) },
        { theme_value::button_focused_border, make_color(20, 10, 20) },
        { theme_value::button_text, make_color(24, 24, 24) },
        { theme_value::button_disabled, make_color(205, 205, 200) },

        { theme_value::input_background, make_color(220, 220, 220) },
        { theme_value::input_text, make_color(25, 25, 20) },
        { theme_value::input_selection, make_color(153, 201, 239) },
        { theme_value::input_cursor, make_color(20, 20, 20) },
        { theme_value::input_border, make_color(40, 120, 140) },
        { theme_value::input_focused_border, make_color(20, 10, 20) },

        { theme_value::tooltip_background, make_color(241, 242, 247) },
        { theme_value::tooltip_border, make_color(118, 118, 118) },
        { theme_value::tooltip_text, make_color(6, 25, 18) }
    },
    dimensions
    {
        { theme_value::window_title_font_size, 18 },
        { theme_value::button_round, 0 },
        { theme_value::button_font_size, 18 },
        { theme_value::input_round, 0 },
        { theme_value::input_font_size, 18 },
        { theme_value::tooltip_text_indent, 5 },
        { theme_value::tooltip_font_size, 16 },
        { theme_value::tooltip_round, 0 },
    },
    strings
    {
#ifdef _WIN32
        { theme_value::window_title_font_name, L"Segoe UI" },
        { theme_value::images_path, L"IMAGES_WHITE" },
        { theme_value::button_font_name, L"Segoe UI" },
        { theme_value::input_font_name, L"Segoe UI" },
        { theme_value::tooltip_font_name, L"Segoe UI" }
#elif __linux__
        { theme_value::window_title_font_name, L"DejaVuSans" },
        { theme_value::images_path, L"IMAGES_WHITE" },
        { theme_value::button_font_name, L"DejaVuSans" },
        { theme_value::input_font_name, L"DejaVuSans" },
        { theme_value::tooltip_font_name, L"DejaVuSans" }
#endif
    }
{
}

void white_theme::set_color(theme_value value_id, color color_)
{
    colors[value_id] = color_;
}

color white_theme::get_color(theme_value value_id) const
{
    auto it = colors.find(value_id);
    if (it != colors.end())
    {
        return it->second;
    }
    return 0;
}

theme white_theme::get_theme() const
{
    return theme::custom;
}

void white_theme::set_dimension(theme_value value_id, int32_t dimension)
{
    dimensions[value_id] = dimension;
}

int32_t white_theme::get_dimension(theme_value value_id) const
{
    auto it = dimensions.find(value_id);
    if (it != dimensions.end())
    {
        return it->second;
    }
    return 0;
}

void white_theme::set_string(theme_value value_id, const std::wstring &value)
{
    strings[value_id] = value;
}

std::wstring white_theme::get_string(theme_value value_id) const
{
    auto it = strings.find(value_id);
    if (it != strings.end())
    {
        return it->second;
    }
    return L"";
}

}
