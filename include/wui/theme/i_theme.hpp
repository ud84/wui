//
// Copyright (c) 2021-2022 Anton Golovkov (udattsk at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/ud84/wui
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

enum class theme_control
{
    window,
    image,
    button,
    input,
    tooltip
};

enum class theme_value
{
    /// colors
    background,
    text,
    calm,
    active,
    border,
    focused_border,
    disabled,
    selection,
    cursor,

    /// strings
    font_name,
    images_path,

    /// ints
    font_size,
    round,
    indent
};

class i_theme
{
public:
    virtual theme get_theme() const = 0;

    virtual void set_color(theme_control control, theme_value value, color color_) = 0;
    virtual color get_color(theme_control control, theme_value value) const = 0;

    virtual void set_dimension(theme_control control, theme_value value, int32_t dimension) = 0;
    virtual int32_t get_dimension(theme_control control, theme_value value) const = 0;

    virtual void set_string(theme_control control, theme_value value, const std::string &str) = 0;
    virtual std::string get_string(theme_control control, theme_value value) const = 0;

    virtual void load_json(const std::string &json) = 0;
    virtual void load_file(const std::string &file_name) = 0;

    virtual ~i_theme() {}
};

}
