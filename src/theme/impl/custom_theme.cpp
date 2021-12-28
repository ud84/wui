//
// Copyright (c) 2021-2022 Anton Golovkov (udattsk at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/ud84/WUI
//

#include <wui/theme/impl/custom_theme.hpp>

namespace wui
{

custom_theme::custom_theme()
    : colors(), dimensions(), strings()
{
}

void custom_theme::set_color(theme_value value_id, color color_)
{
    colors[value_id] = color_;
}

color custom_theme::get_color(theme_value value_id) const
{
    auto it = colors.find(value_id);
    if (it != colors.end())
    {
        return it->second;
    }
    return 0;
}

theme custom_theme::get_theme() const
{
    return theme::custom;
}

void custom_theme::set_dimension(theme_value value_id, int32_t dimension)
{
    dimensions[value_id] = dimension;
}

int32_t custom_theme::get_dimension(theme_value value_id) const
{
    auto it = dimensions.find(value_id);
    if (it != dimensions.end())
    {
        return it->second;
    }
    return 0;
}

void custom_theme::set_string(theme_value value_id, const std::wstring &value)
{
    strings[value_id] = value;
}

std::wstring custom_theme::get_string(theme_value value_id) const
{
    auto it = strings.find(value_id);
    if (it != strings.end())
    {
        return it->second;
    }
    return L"";
}

}
