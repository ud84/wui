#include <WUI/Theme/Theme.h>

#include <WUI/Theme/impl/DarkTheme.h>
#include <WUI/Theme/impl/WhiteTheme.h>
#include <WUI/Theme/impl/CustomTheme.h>

namespace WUI
{

static std::shared_ptr<ITheme> instance = nullptr;

/// Interface

void SetDefaultTheme(Theme theme)
{
	instance.reset();

	switch (theme)
	{
		case Theme::Dark:
			instance = std::shared_ptr<ITheme>(new DarkTheme());
		break;
		case Theme::White:
			instance = std::shared_ptr<ITheme>(new WhiteTheme());
		break;
	}
}

Theme GetDefaultTheme()
{
	if (instance)
	{
		return instance->GetTheme();
	}
	return Theme::Dark;
}

std::shared_ptr<ITheme> MakeCustomTheme()
{
	return std::shared_ptr<CustomTheme>(new CustomTheme());
}

Color ThemeColor(ThemeValue valueID, std::shared_ptr<ITheme> theme)
{
	if (theme)
	{
		return theme->GetColor(valueID);
	}
	else if (instance)
	{
		return instance->GetColor(valueID);
	}
	return 0;
}

int32_t ThemeDimension(ThemeValue valueID, std::shared_ptr<ITheme> theme)
{
	if (theme)
	{
		return theme->GetDimension(valueID);
	}
	else if (instance)
	{
		return instance->GetDimension(valueID);
	}
	return 0;
}

}
