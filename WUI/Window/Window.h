#pragma once

#include <WUI/Window/IWindow.h>
#include <WUI/Common/Rect.h>
#include <WUI/Control/IControl.h>
#include <WUI/Event/Event.h>

#include <vector>
#include <memory>

#ifdef _WIN32
#include <windows.h>
#endif

namespace WUI
{

enum class WindowState
{
	Normal,
	Minimized,
	Maximized
};

class Button;

class Window : public IWindow, public IControl, public std::enable_shared_from_this<Window>
{
public:
	Window();
	~Window();

	/// IWindow
	virtual bool Init(WindowType type, const Rect &position, const std::wstring &caption, std::function<void(void)> closeCallback, std::shared_ptr<ITheme> theme = nullptr);
	virtual void Destroy();

	virtual void AddControl(std::shared_ptr<IControl> control, const Rect &position);
	virtual void RemoveControl(std::shared_ptr<IControl> control);

	virtual void Redraw(const Rect &position, bool clear = false);

	/// IControl
	virtual void Draw(Graphic &gr);

	virtual void ReceiveEvent(const Event &ev); /// <- Events from parent window

	virtual void SetPosition(const Rect &position);
	virtual Rect GetPosition() const;

	virtual void SetParent(std::shared_ptr<Window> window);
	virtual void ClearParent();

	virtual void SetFocus();
	virtual bool RemoveFocus();
	virtual bool Focused() const;
	virtual bool Focusing() const;

	virtual void UpdateTheme(std::shared_ptr<ITheme> theme = nullptr);

	virtual void Show();
	virtual void Hide();
	virtual bool Showed() const;

	virtual void Enable();
	virtual void Disable();
	virtual bool Enabled() const;

	/// Window state methods
	void Minimize();
	void Expand();
	void Normal();
	WindowState GetWindowState() const;

	/// Show/hide window caption and button
	void ShowTitle();
	void HideTitle();

	/// Methods used to block the window while a modal dialog is displayed
	void Block();
	void Unlock();

	/// Callbacks
	void SetSizeChangeCallback(std::function<void(int32_t, int32_t)> sizeChangeCallback);

private:
	std::vector<std::shared_ptr<IControl>> controls;
	std::shared_ptr<IControl> activeControl;

	WindowType windowType;
	Rect position, normalPosition;
	std::wstring caption;
	WindowState windowState;
	std::shared_ptr<ITheme> theme;

	bool showed, enabled, titleShowed;

	size_t focusedIndex;

	std::shared_ptr<Window> parent;

	enum class MovingMode
	{
		Move,
		SizeWELeft,
		SizeWERight,
		SizeNSTop,
		SizeNSBottom,
		SizeNWSETop,
		SizeNWSEBottom,
		SizeNESWTop,
		SizeNESWBottom
	};
	MovingMode movingMode;

	std::function<void(void)> closeCallback;
	std::function<void(int32_t, int32_t)> sizeChangeCallback;

	std::shared_ptr<ITheme> buttonsTheme, closeButtonTheme;
	std::shared_ptr<Button> minimizeButton, expandButton, closeButton;

#ifdef _WIN32
	HWND hWnd;

	HBRUSH backgroundBrush;
	HFONT font;

	int16_t xClick, yClick;
	bool mouseTracked;

	static LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

	void MakePrimitives();
	void DestroyPrimitives();

	void UpdatePosition();
#endif

	void SendMouseEvent(const MouseEvent &ev);
	void ChangeFocus();
	void ExecuteFocused();
	void SetFocused(std::shared_ptr<IControl> &control);

	void UpdateControlButtonsTheme();
};

}
