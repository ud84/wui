#pragma once

#include <WUI/Window/Type.h>
#include <WUI/Event/Event.h>
#include <WUI/Control/IControl.h>

#include <functional>

#include <string>

namespace WUI
{

class IWindow
{
public:
	virtual bool Init(WindowType type, const Rect &position, const std::string &caption, std::function<void(void)> closeCallback) = 0;
	virtual void Destroy() = 0;

	virtual void AddControl(std::shared_ptr<IControl> control, const Rect &position) = 0;
	virtual void RemoveControl(std::shared_ptr<IControl> control) = 0;
	virtual void Redraw(const Rect &position) = 0;

protected:
	~IWindow() {}

};

}
