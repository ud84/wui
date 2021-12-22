#include <WUI/Theme/impl/WhiteTheme.h>

namespace WUI
{

WhiteTheme::WhiteTheme()
	: colors
	{
		{ ThemeValue::Window_Background, MakeColor(240, 240, 240) },
		{ ThemeValue::Window_Text, MakeColor(25, 25, 20) },
		{ ThemeValue::Button_Calm, MakeColor(6, 165, 223) },
		{ ThemeValue::Button_Active, MakeColor(26, 175, 233) },
		{ ThemeValue::Button_Border, MakeColor(0, 160, 210) },
		{ ThemeValue::Button_Text, MakeColor(24, 24, 24) },
		{ ThemeValue::Button_Disabled, MakeColor(205, 205, 200) }
	},
	dimensions
	{
		{ThemeValue::Button_Round, 5}
	}
{
}

void WhiteTheme::SetColor(ThemeValue valueID, Color color)
{
	auto it = colors.find(valueID);
	if (it != colors.end())
	{
		it->second = color;
	}
}

Color WhiteTheme::GetColor(ThemeValue valueID) const
{
	auto it = colors.find(valueID);
	if (it != colors.end())
	{
		return it->second;
	}
	return 0;
}

Theme WhiteTheme::GetTheme() const
{
	return Theme::White;
}

void WhiteTheme::SetDimension(ThemeValue valueID, int32_t dimension)
{
	auto it = dimensions.find(valueID);
	if (it != dimensions.end())
	{
		it->second = dimension;
	}
}

int32_t WhiteTheme::GetDimension(ThemeValue valueID) const
{
	auto it = dimensions.find(valueID);
	if (it != dimensions.end())
	{
		return it->second;
	}
	return 0;
}

}
