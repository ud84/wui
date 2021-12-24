#pragma once

#include <WUI/Common/Color.h>

#include <cstdint>

namespace WUI
{

enum class Theme
{
	Dark,
	White,
	Custom
};

enum class ThemeValue
{
	Window_Background,
	Window_Text,
	Window_ActiveButton,

	Button_Calm,
	Button_Active,
	Button_Border,
	Button_FocusedBorder,
	Button_Text,
	Button_Disabled,
	Button_Round
};

class ITheme
{
public:
	virtual void SetColor(ThemeValue valueID, Color color) = 0;
	virtual Color GetColor(ThemeValue valueID) const = 0;

	virtual Theme GetTheme() const = 0;

	virtual void SetDimension(ThemeValue valueID, int32_t dimension) = 0;
	virtual int32_t GetDimension(ThemeValue valueID) const = 0;

	virtual ~ITheme() {}
};

}
