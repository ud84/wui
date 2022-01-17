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

namespace wui
{

static std::shared_ptr<i_theme> instance = nullptr;
static std::string dummy_string;
static std::vector<uint8_t> dummy_image;

/// Interface

void set_default_theme_from_json(const std::string &name, const std::string &json, bool dark)
{
    instance.reset();
    instance = std::shared_ptr<i_theme>(new theme_impl(name, dark));
    instance->load_json(json);
}

void set_default_theme_from_file(const std::string &name, const std::string &file_name, bool dark)
{
    instance.reset();
    instance = std::shared_ptr<i_theme>(new theme_impl(name, dark));
    instance->load_file(file_name);
}

void set_default_theme_empty(const std::string &name, bool dark)
{
    instance.reset();
    instance = std::shared_ptr<i_theme>(new theme_impl(name, dark));
}

std::shared_ptr<i_theme> get_default_theme()
{
    return instance;    
}

bool is_dark_default_theme()
{
    if (instance)
    {
        return instance->get_dark();
    }
    return false;
}

std::shared_ptr<i_theme> make_custom_theme(const std::string &name, bool dark)
{
    return std::shared_ptr<i_theme>(new theme_impl(name, dark));
}

std::shared_ptr<i_theme> make_custom_theme(const std::string &name, const std::string &json, bool dark)
{
    auto ct = std::shared_ptr<i_theme>(new theme_impl(name, dark));
    ct->load_json(json);
    return ct;
}

color theme_color(const std::string &control, const std::string &value, std::shared_ptr<i_theme> theme_)
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

int32_t theme_dimension(const std::string &control, const std::string &value, std::shared_ptr<i_theme> theme_)
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

const std::string &theme_string(const std::string &control, const std::string &value, std::shared_ptr<i_theme> theme_)
{
    if (theme_)
    {
        return theme_->get_string(control, value);
    }
    else if (instance)
    {
        return instance->get_string(control, value);
    }
    return dummy_string;
}

font theme_font(const std::string &control, const std::string &value, std::shared_ptr<i_theme> theme_)
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

const std::vector<uint8_t> &theme_image(const std::string &name, std::shared_ptr<i_theme> theme_)
{
    if (theme_)
    {
        return theme_->get_image(name);
    }
    else if (instance)
    {
        return instance->get_image(name);
    }

    return dummy_image;
}

}
