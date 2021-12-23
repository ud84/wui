#include <WUI/Theme/impl/CustomTheme.h>

namespace WUI
{

CustomTheme::CustomTheme()
	: colors(), dimensions()
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

}
