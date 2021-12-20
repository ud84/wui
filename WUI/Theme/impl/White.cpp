#include <WUI/Theme/impl/White.h>

namespace WUI
{

White::White()
	: tv
	{
		{ ThemeValue::Window_Background, MakeColor(240, 240, 240) },
		{ ThemeValue::Window_Text, MakeColor(25, 25, 20) },
		{ ThemeValue::Button_Calm, MakeColor(6, 165, 223) },
		{ ThemeValue::Button_Active, MakeColor(26, 175, 233) },
		{ ThemeValue::Button_Border, MakeColor(0, 160, 210) },
		{ ThemeValue::Button_Text, MakeColor(24, 24, 24) },
		{ ThemeValue::Button_Disabled, MakeColor(205, 205, 200) }
	}
{
}

const Color White::GetColor(ThemeValue valueID)
{
	auto it = tv.find(valueID);
	if (it != tv.end())
	{
		return it->second;
	}
	return 0;
}

const Theme White::GetTheme()
{
	return Theme::White;
}

}
