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
#include <wui/common/font.hpp>

#include <cstdint>
#include <string>
#include <vector>

namespace wui
{

class i_theme
{
public:
    virtual std::string get_name() const = 0;

    virtual void set_color(const std::string &control, const std::string &value, color color_) = 0;
    virtual color get_color(const std::string &control, const std::string &value) const = 0;

    virtual void set_dimension(const std::string &control, const std::string &value, int32_t dimension) = 0;
    virtual int32_t get_dimension(const std::string &control, const std::string &value) const = 0;

    virtual void set_string(const std::string &control, const std::string &value, const std::string &str) = 0;
    virtual const std::string &get_string(const std::string &control, const std::string &value) const = 0;

    virtual void set_font(const std::string &control, const std::string &value, const font &font_) = 0;
    virtual font get_font(const std::string &control, const std::string &value) const = 0;

    virtual void set_image(const std::string &name, const std::vector<uint8_t> &data) = 0;
    virtual const std::vector<uint8_t> &get_image(const std::string &name) = 0;

#ifdef _WIN32
    virtual void load_resource(int32_t resource_index, const std::string &resource_section) = 0;
#endif
    virtual void load_json(const std::string &json) = 0;
    virtual void load_file(const std::string &file_name) = 0;
    virtual void load_theme(const i_theme &theme_) = 0;

    virtual bool is_ok() const = 0;

    virtual ~i_theme() {}
};

}
