#pragma once

#include <WUI/Theme/ITheme.h>

#include <cstdint>

#include <memory>

namespace WUI
{

/// Set and get the current theme
void SetDefaultTheme(Theme theme);
Theme GetDefaultTheme();

std::shared_ptr<ITheme> MakeCustomTheme();

/// Return the item's color by current color theme
Color ThemeColor(ThemeValue valueID, std::shared_ptr<ITheme> theme = nullptr);

int32_t ThemeDimension(ThemeValue valueID, std::shared_ptr<ITheme> theme = nullptr);

}
