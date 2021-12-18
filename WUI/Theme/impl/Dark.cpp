#include <WUI/Theme/impl/Dark.h>

namespace WUI
{

Dark::Dark()
	: tv
	{
		{ ThemeValue::Window_Background, MakeColor(10, 10, 10) }
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
