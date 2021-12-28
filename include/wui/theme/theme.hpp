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

#include <cstdint>

#include <memory>

namespace wui
{

/// Set and get the current theme
void set_default_theme(theme theme_);
theme get_default_theme();

std::shared_ptr<i_theme> make_custom_theme();

/// Return the item's color by current color theme
color theme_color(theme_value value_id, std::shared_ptr<i_theme> theme_ = nullptr);

/// Return the item's dimension by current color theme
int32_t theme_dimension(theme_value value_id, std::shared_ptr<i_theme> theme_ = nullptr);

/// Return the item's string value by current color theme
std::wstring theme_string(theme_value value_id, std::shared_ptr<i_theme> theme_ = nullptr);

}
