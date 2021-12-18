#pragma once

#include <WUI/Theme/Theme.h>

namespace WUI
{

class ITheme
{
public:
	virtual const Color GetColor(ThemeValue valueID) = 0;

	virtual const Theme GetTheme() = 0;

	virtual ~ITheme() {}
};

}
