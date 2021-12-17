#pragma once

#include <WUI/Window/Type.h>
#include <WUI/Window/IWindow.h>
#include <WUI/Common/Rect.h>
#include <WUI/Control/IControl.h>

namespace WUI
{

class Window : public IWindow
{
public:
	Window(WindowType type, Rect &position);

	virtual void AddControl(IControl &control, Rect &position);
	virtual void RemoveControl(IControl &control);

	virtual void ReceiveEvent(Event &ev);

	virtual void Show();
	virtual void Hide();
};

}
