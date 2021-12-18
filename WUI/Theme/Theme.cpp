#include <WUI/Theme/Theme.h>

#include <WUI/Theme/impl/Dark.h>
#include <WUI/Theme/impl/White.h>

namespace WUI
{

static ITheme* instance = nullptr;

/// Interface

void SetTheme(Theme theme)
{
	delete instance;

	switch (theme)
	{
		case Theme::Dark:
			instance = new Dark();
		break;
		case Theme::White:
			instance = new White();
		break;
	}
}

Theme GetTheme()
{
	if (instance)
	{
		return instance->GetTheme();
	}
	return Theme::Dark;
}

Color ThemeColor(ThemeValue valueID)
{
	if (instance)
	{
		return instance->GetColor(valueID);
	}
	return 0;
}

}
