//
// Copyright (c) 2021-2022 Anton Golovkov (udattsk at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/ud84/WUI
//

#pragma once

#include <wui/common/color.hpp>

#include <cstdint>
#include <string>

namespace WUI
{

enum class Theme
{
	Dark,
	White,
	Custom
};

enum class ThemeValue
{
	Window_Background,
	Window_Text,
	Window_ActiveButton,

	Images_Path,

	Button_Calm,
	Button_Active,
	Button_Border,
	Button_FocusedBorder,
	Button_Text,
	Button_Disabled,
	Button_Round
};

class ITheme
{
public:
	virtual void SetColor(ThemeValue valueID, Color color) = 0;
	virtual Color GetColor(ThemeValue valueID) const = 0;

	virtual Theme GetTheme() const = 0;

	virtual void SetDimension(ThemeValue valueID, int32_t dimension) = 0;
	virtual int32_t GetDimension(ThemeValue valueID) const = 0;

	virtual void SetString(ThemeValue valueID, const std::wstring &value) = 0;
	virtual std::wstring GetString(ThemeValue valueID) const = 0;

	virtual ~ITheme() {}
};

}
