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

std::shared_ptr<i_theme> get_default_theme()
{
    return instance;    
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

}
