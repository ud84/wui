//
// Copyright (c) 2021-2022 Anton Golovkov (udattsk at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/ud84/WUI
//

#pragma once

#include <wui/control/icontrol.hpp>
#include <wui/theme/itheme.hpp>

#include <functional>

#include <string>

namespace WUI
{

enum class WindowType
{
	Frame,
	Dialog
};

class IWindow
{
public:
	virtual bool Init(WindowType type, const Rect &position, const std::wstring &caption, std::function<void(void)> closeCallback, std::shared_ptr<ITheme> theme = nullptr) = 0;
	virtual void Destroy() = 0;

	virtual void AddControl(std::shared_ptr<IControl> control, const Rect &position) = 0;
	virtual void RemoveControl(std::shared_ptr<IControl> control) = 0;

	virtual void Redraw(const Rect &position, bool clear = false) = 0;

protected:
	~IWindow() {}

};

}
