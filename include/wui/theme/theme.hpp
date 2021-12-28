//
// Copyright (c) 2021-2022 Anton Golovkov (udattsk at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/ud84/WUI
//

#pragma once

#include <wui/theme/itheme.hpp>

#include <cstdint>

#include <memory>

namespace WUI
{

/// Set and get the current theme
void SetDefaultTheme(Theme theme);
Theme GetDefaultTheme();

std::shared_ptr<ITheme> MakeCustomTheme();

/// Return the item's color by current color theme
Color ThemeColor(ThemeValue valueID, std::shared_ptr<ITheme> theme = nullptr);

/// Return the item's dimension by current color theme
int32_t ThemeDimension(ThemeValue valueID, std::shared_ptr<ITheme> theme = nullptr);

/// Return the item's string value by current color theme
std::wstring ThemeString(ThemeValue valueID, std::shared_ptr<ITheme> theme = nullptr);

}
