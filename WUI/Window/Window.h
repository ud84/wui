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

class Window : public IWindow, public IControl, public std::enable_shared_from_this<Window>
{
public:
	Window();
	~Window();

	/// IWindow
	virtual bool Init(WindowType type, const Rect &position, const std::string &caption, std::function<void(void)> closeCallback);
	virtual void Destroy();

	virtual void AddControl(std::shared_ptr<IControl> control, const Rect &position);
	virtual void RemoveControl(std::shared_ptr<IControl> control);
	virtual void Redraw(const Rect &position);

	/// IControl
	virtual void Draw(Graphic &gr);

	virtual void ReceiveEvent(const Event &ev); /// <- Events from parent window

	virtual void SetPosition(const Rect &position);
	virtual Rect GetPosition() const;

	virtual void SetParent(std::shared_ptr<Window> window);
	virtual void ClearParent();

	virtual void UpdateTheme();

	virtual void Show();
	virtual void Hide();

private:
	std::vector<std::shared_ptr<IControl>> controls;
	std::shared_ptr<IControl> activeControl;

	Rect position;
	std::string caption;
	std::function<void(void)> closeCallback;

	bool showed;

	std::shared_ptr<Window> parent;

#ifdef _WIN32
	HWND hWnd;

	HBRUSH backgroundBrush;
	HFONT font;

	int16_t xClick, yClick;

	static LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

	void MakePrimitives();
	void DestroyPrimitives();
#endif

	void SendMouseEvent(const MouseEvent &ev);
};

}
