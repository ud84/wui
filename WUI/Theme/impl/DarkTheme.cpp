#include <WUI/Theme/impl/DarkTheme.h>

namespace WUI
{

DarkTheme::DarkTheme()
	: colors
	{
		{ ThemeValue::Window_Background, MakeColor(19, 21, 25) },
		{ ThemeValue::Window_Text, MakeColor(245, 245, 240) },
		{ ThemeValue::Button_Calm, MakeColor(6, 165, 223) },
		{ ThemeValue::Button_Active, MakeColor(26, 175, 233) },
		{ ThemeValue::Button_Border, MakeColor(0, 160, 210) },
		{ ThemeValue::Button_Text, MakeColor(240, 241, 241) },
		{ ThemeValue::Button_Disabled, MakeColor(165, 165, 160) }
	},
	dimensions
	{
		{ ThemeValue::Button_Round, 5}
	}
{
}

void DarkTheme::SetColor(ThemeValue valueID, Color color)
{
	auto it = colors.find(valueID);
	if (it != colors.end())
	{
		it->second = color;
	}
}

Color DarkTheme::GetColor(ThemeValue valueID) const
{
	auto it = colors.find(valueID);
	if (it != colors.end())
	{
		return it->second;
	}
	return 0;
}

Theme DarkTheme::GetTheme() const
{
	return Theme::Dark;
}

void DarkTheme::SetDimension(ThemeValue valueID, int32_t dimension)
{
	auto it = dimensions.find(valueID);
	if (it != dimensions.end())
	{
		it->second = dimension;
	}
}

int32_t DarkTheme::GetDimension(ThemeValue valueID) const
{
	auto it = dimensions.find(valueID);
	if (it != dimensions.end())
	{
		return it->second;
	}
	return 0;
}

}
