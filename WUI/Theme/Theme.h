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
	Window_Text,

	Button_Calm,
	Button_Active,
	Button_Border,
	Button_Text,
	Button_Disabled,

	WC_Button_Calm,
	WC_Button_Active
};

/// Set and get the current theme
void SetTheme(Theme theme);
Theme GetTheme();

/// Return the item's color by current color theme
Color ThemeColor(ThemeValue valueID);

}
