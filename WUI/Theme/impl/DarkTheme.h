#pragma once

#include <WUI/Theme/ITheme.h>

#include <map>

namespace WUI
{

class DarkTheme : public ITheme
{
public:
	DarkTheme();

	virtual void SetColor(ThemeValue valueID, Color color);
	virtual Color GetColor(ThemeValue valueID) const;

	virtual Theme GetTheme() const;

	virtual void SetDimension(ThemeValue valueID, int32_t dimension);
	virtual int32_t GetDimension(ThemeValue valueID) const;

private:
	std::map<ThemeValue, Color> colors;
	std::map<ThemeValue, int32_t> dimensions;
};

}
