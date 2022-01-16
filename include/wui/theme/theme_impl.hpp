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
    theme_impl(const std::string &name);

    virtual std::string get_name() const;

    virtual void set_color(const std::string &control, const std::string &value, color color_);
    virtual color get_color(const std::string &control, const std::string &value) const;

    virtual void set_dimension(const std::string &control, const std::string &value, int32_t dimension);
    virtual int32_t get_dimension(const std::string &control, const std::string &value) const;

    virtual void set_string(const std::string &control, const std::string &value, const std::string &str);
    virtual std::string get_string(const std::string &control, const std::string &value) const;

    virtual void set_font(const std::string &control, const std::string &value, const font &font_);
    virtual font get_font(const std::string &control, const std::string &value) const;

    virtual void load_json(const std::string &json);
    virtual void load_file(const std::string &file_name);
    virtual void load_theme(const i_theme &theme_);

private:
    std::string name;

    std::map<std::string, int32_t> ints;
    std::map<std::string, std::string> strings;
    std::map<std::string, font> fonts;

};

}
