#pragma once

#include <WUI/Theme/impl/ITheme.h>

#include <map>

namespace WUI
{

class Dark : public ITheme
{
public:
	Dark();

	virtual const Color GetColor(ThemeValue valueID);

	virtual const Theme GetTheme();

private:
	std::map<ThemeValue, Color> tv;
};

}
