//
// Copyright (c) 2021-2022 Anton Golovkov (udattsk at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/ud84/WUI
//

#include <wui/theme/theme.hpp>

#include <wui/theme/impl/dark_theme.hpp>
#include <wui/theme/impl/white_theme.hpp>
#include <wui/theme/impl/custom_theme.hpp>

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
            instance = std::shared_ptr<i_theme>(new dark_theme());
        break;
        case theme::white:
            instance = std::shared_ptr<i_theme>(new white_theme());
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
    return std::shared_ptr<custom_theme>(new custom_theme());
}

color theme_color(theme_value value_id, std::shared_ptr<i_theme> theme_)
{
    if (theme_)
    {
        return theme_->get_color(value_id);
    }
    else if (instance)
	{
        return instance->get_color(value_id);
    }
    return 0;
}

int32_t theme_dimension(theme_value value_id, std::shared_ptr<i_theme> theme_)
{
    if (theme_)
    {
        return theme_->get_dimension(value_id);
    }
    else if (instance)
    {
        return instance->get_dimension(value_id);
    }
    return 0;
}

std::wstring theme_string(theme_value value_id, std::shared_ptr<i_theme> theme_)
{
    if (theme_)
    {
        return theme_->get_string(value_id);
    }
    else if (instance)
    {
        return instance->get_string(value_id);
    }
    return 0;
}

}
