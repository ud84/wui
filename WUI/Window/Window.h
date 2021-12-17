#pragma once

#include <WUI/Window/Type.h>
#include <WUI/Window/IWindow.h>
#include <WUI/Common/Rect.h>
#include <WUI/Control/IControl.h>

#include <vector>
#include <memory>

#ifdef _WIN32
#include <windows.h>
#endif

namespace WUI
{

class Window : public IWindow
{
public:
	Window();
	~Window();

	virtual void Init(WindowType type, const Rect &position, const std::string &caption);

	virtual void AddControl(IControl &control, const Rect &position);
	virtual void RemoveControl(IControl &control);

	virtual void ReceiveEvent(Event &ev);

	virtual void Show();
	virtual void Hide();

private:
	std::vector<IControl*> controls;

#ifdef _WIN32
	HWND hWnd;

	void MyRegisterClass();
	static LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
#endif
};

}
