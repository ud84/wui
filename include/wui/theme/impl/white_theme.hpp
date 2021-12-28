//
// Copyright (c) 2021-2022 Anton Golovkov (udattsk at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/ud84/WUI
//

#pragma once

#include <wui/theme/itheme.hpp>

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
