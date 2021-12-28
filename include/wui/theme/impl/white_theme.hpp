//
// Copyright (c) 2021-2022 Anton Golovkov (udattsk at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/ud84/WUI
//

#pragma once

#include <wui/theme/i_theme.hpp>

#include <map>

namespace wui
{

class white_theme : public i_theme
{
public:
    white_theme();

    virtual void set_color(theme_value value_id, color color_);
    virtual color get_color(theme_value value_id) const;

    virtual theme get_theme() const;

    virtual void set_dimension(theme_value value_id, int32_t dimension);
    virtual int32_t get_dimension(theme_value value_id) const;

    virtual void set_string(theme_value value_id, const std::wstring &value);
    virtual std::wstring get_string(theme_value value_id) const;

private:
    std::map<theme_value, color> colors;
    std::map<theme_value, int32_t> dimensions;
    std::map<theme_value, std::wstring> strings;
};

}
