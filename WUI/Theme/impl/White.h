#pragma once

#include <WUI/Theme/impl/ITheme.h>

#include <map>

namespace WUI
{

class White : public ITheme
{
public:
	White();

	virtual const Color GetColor(ThemeValue valueID);

	virtual const Theme GetTheme();

private:
	std::map<ThemeValue, Color> tv;
};

}
