//
// Copyright (c) 2021-2022 Anton Golovkov (udattsk at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/ud84/wui
//

#pragma once

#include <wui/theme/i_theme.hpp>

#include <map>

namespace wui
{

class theme_impl : public i_theme
{
public:
    theme_impl(theme theme_);

    virtual theme get_theme() const;

    virtual void set_color(theme_control control, theme_value value, color color_);
    virtual color get_color(theme_control control, theme_value value) const;

    virtual void set_dimension(theme_control control, theme_value value, int32_t dimension);
    virtual int32_t get_dimension(theme_control control, theme_value value) const;

    virtual void set_string(theme_control control, theme_value value, const std::string &str);
    virtual std::string get_string(theme_control control, theme_value value) const;

    virtual void set_font(theme_control control, theme_value value, const font &font_);
    virtual font get_font(theme_control control, theme_value value) const;

    virtual void load_json(const std::string &json);
    virtual void load_file(const std::string &file_name);
    virtual void load_theme(const i_theme &theme_);

private:
    theme theme_;

    std::map<std::string, int32_t> ints;
    std::map<std::string, std::string> strings;
    std::map<std::string, font> fonts;

    friend class theme_impl;
};

}
