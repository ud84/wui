//
// Copyright (c) 2021-2022 Anton Golovkov (udattsk at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/ud84/WUI
//

#include <wui/theme/theme_impl.hpp>
#include <wui/theme/theme_str.hpp>

namespace wui
{

theme_impl::theme_impl(theme theme__)
    : theme_(theme__), ints(), strings()
{
}

theme theme_impl::get_theme() const
{
    return theme_;
}

void theme_impl::set_color(theme_control control, theme_value value, color color_)
{
    ints[str_theme_control(control) + str_theme_value(value)] = color_;
}

color theme_impl::get_color(theme_control control, theme_value value) const
{
    auto it = ints.find(str_theme_control(control) + str_theme_value(value));
    if (it != ints.end())
    {
        return static_cast<color>(it->second);
    }
    return 0;
}

void theme_impl::set_dimension(theme_control control, theme_value value, int32_t dimension)
{
    ints[str_theme_control(control) + str_theme_value(value)] = dimension;
}

int32_t theme_impl::get_dimension(theme_control control, theme_value value) const
{
    auto it = ints.find(str_theme_control(control) + str_theme_value(value));
    if (it != ints.end())
    {
        return it->second;
    }
    return 0;
}

void theme_impl::set_string(theme_control control, theme_value value, const std::string &str)
{
    strings[str_theme_control(control) + str_theme_value(value)] = str;
}

std::string theme_impl::get_string(theme_control control, theme_value value) const
{
    auto it = strings.find(str_theme_control(control) + str_theme_value(value));
    if (it != strings.end())
    {
        return it->second;
    }
    return "";
}

}
