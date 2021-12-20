#include <WUI/Theme/impl/Dark.h>

namespace WUI
{

Dark::Dark()
	: tv
	{
		{ ThemeValue::Window_Background, MakeColor(19, 21, 25) },
		{ ThemeValue::Window_Text, MakeColor(245, 245, 240) },
		{ ThemeValue::Button_Calm, MakeColor(6, 165, 223) },
		{ ThemeValue::Button_Active, MakeColor(26, 175, 233) },
		{ ThemeValue::Button_Border, MakeColor(0, 160, 210) },
		{ ThemeValue::Button_Text, MakeColor(240, 241, 241) },
		{ ThemeValue::Button_Disabled, MakeColor(165, 165, 160) }
	}
{
}

const Color Dark::GetColor(ThemeValue valueID)
{
	auto it = tv.find(valueID);
	if (it != tv.end())
	{
		return it->second;
	}
	return 0;
}

const Theme Dark::GetTheme()
{
	return Theme::Dark;
}

}
