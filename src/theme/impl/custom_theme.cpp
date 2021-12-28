//
// Copyright (c) 2021-2022 Anton Golovkov (udattsk at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/ud84/WUI
//

#include <wui/theme/impl/custom_theme.hpp>

namespace WUI
{

CustomTheme::CustomTheme()
	: colors(), dimensions(), strings()
{
}

void CustomTheme::SetColor(ThemeValue valueID, Color color)
{
	colors[valueID] = color;
}

Color CustomTheme::GetColor(ThemeValue valueID) const
{
	auto it = colors.find(valueID);
	if (it != colors.end())
	{
		return it->second;
	}
	return 0;
}

Theme CustomTheme::GetTheme() const
{
	return Theme::Custom;
}

void CustomTheme::SetDimension(ThemeValue valueID, int32_t dimension)
{
	dimensions[valueID] = dimension;
}

int32_t CustomTheme::GetDimension(ThemeValue valueID) const
{
	auto it = dimensions.find(valueID);
	if (it != dimensions.end())
	{
		return it->second;
	}
	return 0;
}

void CustomTheme::SetString(ThemeValue valueID, const std::wstring &value)
{
	strings[valueID] = value;
}

std::wstring CustomTheme::GetString(ThemeValue valueID) const
{
	auto it = strings.find(valueID);
	if (it != strings.end())
	{
		return it->second;
	}
	return L"";
}

}
