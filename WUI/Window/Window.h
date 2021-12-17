#pragma once

#include <WUI/Window/Type.h>
#include <WUI/Window/IWindow.h>
#include <WUI/Common/Rect.h>
#include <WUI/Control/IControl.h>

#include <vector>
#include <memory>

namespace WUI
{

class Window : public IWindow
{
public:
	Window(WindowType type, const Rect &position);

	virtual void AddControl(IControl &control, const Rect &position);
	virtual void RemoveControl(IControl &control);

	virtual void ReceiveEvent(Event &ev);

	virtual void Show();
	virtual void Hide();

private:
	std::vector<IControl*> controls;
};

}
