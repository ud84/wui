//
// Copyright (c) 2021-2022 Anton Golovkov (udattsk at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/ud84/wui
//

#include <wui/theme/theme.hpp>
#include <wui/theme/theme_impl.hpp>
#include <wui/theme/theme_str.hpp>

namespace wui
{

static std::shared_ptr<i_theme> instance = nullptr;

/// Interface

void set_default_theme(theme theme_)
{
    instance.reset();

    switch (theme_)
    {
        case theme::dark:
            instance = std::shared_ptr<i_theme>(new theme_impl(theme::dark));
            instance->load_json(dark_json);
        break;
        case theme::white:
            instance = std::shared_ptr<i_theme>(new theme_impl(theme::white));
            instance->load_json(white_json);
        break;
    }
}

theme get_default_theme()
{
    if (instance)
    {
        return instance->get_theme();
    }
    return theme::dark;
}

std::shared_ptr<i_theme> make_custom_theme()
{
    return std::shared_ptr<i_theme>(new theme_impl(theme::custom));
}

color theme_color(theme_control control, theme_value value, std::shared_ptr<i_theme> theme_)
{
    if (theme_)
    {
        return theme_->get_color(control, value);
    }
    else if (instance)
    {
        return instance->get_color(control, value);
    }
    return 0;
}

int32_t theme_dimension(theme_control control, theme_value value, std::shared_ptr<i_theme> theme_)
{
    if (theme_)
    {
        return theme_->get_dimension(control, value);
    }
    else if (instance)
    {
        return instance->get_dimension(control, value);
    }
    return 0;
}

std::string theme_string(theme_control control, theme_value value, std::shared_ptr<i_theme> theme_)
{
    if (theme_)
    {
        return theme_->get_string(control, value);
    }
    else if (instance)
    {
        return instance->get_string(control, value);
    }
    return 0;
}

font theme_font(theme_control control, theme_value value, std::shared_ptr<i_theme> theme_)
{
    if (theme_)
    {
        return theme_->get_font(control, value);
    }
    else if (instance)
    {
        return instance->get_font(control, value);
    }
    return font();
}

std::string str_theme_control(theme_control tc)
{
    switch (tc)
    {
        case theme_control::window:
            return "window";
        break;
        case theme_control::image:
            return "image";
        break;
        case theme_control::button:
            return "button";
        break;
        case theme_control::input:
            return "input";
        break;
        case theme_control::tooltip:
            return "tooltip";
        break;
    }
    return "";
}

std::string str_theme_value(theme_value tv)
{
    switch (tv)
    {
        case theme_value::background:
            return "background";
        break;
        case theme_value::text:
            return "text";
        break;
        case theme_value::calm:
            return "calm";
        break;
        case theme_value::active:
            return "active";
        break;
        case theme_value::border:
            return "border";
        break;
        case theme_value::focused_border:
            return "focused_border";
        break;
        case theme_value::disabled:
            return "disabled";
        break;
        case theme_value::selection:
            return "selection";
        break;
        case theme_value::cursor:
            return "cursor";
        break;
        case theme_value::font:
            return "font";
        break;
        case theme_value::images_path:
            return "images_path";
        break;
        case theme_value::round:
            return "round";
        break;
        case theme_value::text_indent:
            return "text_indent";
        break;
    }
    return "";
}

}
