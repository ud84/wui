#pragma once

#include <cstdint>
#include <WUI/Common/Color.h>

namespace WUI
{

enum class Theme
{
	Dark,
	White
};

enum class ThemeValue
{
	Window_Background,
	Window_Caption,

	Button_Calm,
	Button_Active,
	Button_Border,
	Button_Text
};

/// Set and get the current theme
void SetTheme(Theme theme);
Theme GetTheme();

/// Return the item's color by current color theme
Color ThemeColor(ThemeValue valueID);

}
