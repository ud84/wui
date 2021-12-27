#pragma once

#include <WUI/Control/IControl.h>
#include <WUI/Theme/ITheme.h>

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
