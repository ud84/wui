#include <WUI/Window/Window.h>

#include <algorithm>

namespace WUI
{

Window::Window(WindowType type, const Rect &position)
	: controls()
{
}

void Window::AddControl(IControl &control, const Rect &position)
{
	//if (std::find(controls.begin(), controls.end(), control) == controls.end())
	{
		//controls.emplace_back(&control);
	}
}

void Window::RemoveControl(IControl &control)
{
	//auto exist = std::find(controls.begin(), controls.end(), controls);
	//if (exist != controls.end())
	{
		//controls.erase(exist);
	}
}

void Window::ReceiveEvent(Event &ev)
{

}

void Window::Show()
{

}

void Window::Hide()
{

}

}
