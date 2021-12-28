//
// Copyright (c) 2021-2022 Anton Golovkov (udattsk at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/ud84/WUI
//

#include <wui/theme/theme.hpp>

#include <wui/theme/impl/dark_theme.hpp>
#include <wui/theme/impl/white_theme.hpp>
#include <wui/theme/impl/custom_theme.hpp>

namespace WUI
{

static std::shared_ptr<ITheme> instance = nullptr;

/// Interface

void SetDefaultTheme(Theme theme)
{
	instance.reset();

	switch (theme)
	{
		case Theme::Dark:
			instance = std::shared_ptr<ITheme>(new DarkTheme());
		break;
		case Theme::White:
			instance = std::shared_ptr<ITheme>(new WhiteTheme());
		break;
	}
}

Theme GetDefaultTheme()
{
	if (instance)
	{
		return instance->GetTheme();
	}
	return Theme::Dark;
}

std::shared_ptr<ITheme> MakeCustomTheme()
{
	return std::shared_ptr<CustomTheme>(new CustomTheme());
}

Color ThemeColor(ThemeValue valueID, std::shared_ptr<ITheme> theme)
{
	if (theme)
	{
		return theme->GetColor(valueID);
	}
	else if (instance)
	{
		return instance->GetColor(valueID);
	}
	return 0;
}

int32_t ThemeDimension(ThemeValue valueID, std::shared_ptr<ITheme> theme)
{
	if (theme)
	{
		return theme->GetDimension(valueID);
	}
	else if (instance)
	{
		return instance->GetDimension(valueID);
	}
	return 0;
}

std::wstring ThemeString(ThemeValue valueID, std::shared_ptr<ITheme> theme)
{
	if (theme)
	{
		return theme->GetString(valueID);
	}
	else if (instance)
	{
		return instance->GetString(valueID);
	}
	return 0;
}

}
