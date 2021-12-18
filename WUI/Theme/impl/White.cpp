#include <WUI/Theme/impl/White.h>

namespace WUI
{

White::White()
	: tv
	{
		{ ThemeValue::Window_Background, MakeColor(240, 240, 240) }
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
