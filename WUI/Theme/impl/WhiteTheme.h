#pragma once

#include <WUI/Theme/ITheme.h>

#include <map>

namespace WUI
{

class WhiteTheme : public ITheme
{
public:
	WhiteTheme();

	virtual void SetColor(ThemeValue valueID, Color color);
	virtual Color GetColor(ThemeValue valueID) const;

	virtual Theme GetTheme() const;

	virtual void SetDimension(ThemeValue valueID, int32_t dimension);
	virtual int32_t GetDimension(ThemeValue valueID) const;

	virtual void SetString(ThemeValue valueID, const std::wstring &value);
	virtual std::wstring GetString(ThemeValue valueID) const;

private:
	std::map<ThemeValue, Color> colors;
	std::map<ThemeValue, int32_t> dimensions;
	std::map<ThemeValue, std::wstring> strings;
};

}
