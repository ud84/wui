#pragma once

#include <WUI/Window/Type.h>
#include <WUI/Event/Event.h>
#include <WUI/Control/IControl.h>

#include <string>

namespace WUI
{

class IWindow
{
public:
	virtual bool Init(WindowType type, const Rect &position, const std::string &caption) = 0;

	virtual void AddControl(IControl &control, const Rect &position) = 0;
	virtual void RemoveControl(IControl &control) = 0;
	virtual void Redraw(const Rect &position) = 0;

	virtual void UpdateTheme() = 0;

	virtual void Show() = 0;
	virtual void Hide() = 0;

protected:
	~IWindow() {}

};

}
