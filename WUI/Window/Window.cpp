#include <WUI/Window/Window.h>

#include <WUI/Graphic/Graphic.h>

#include <WUI/Theme/Theme.h>

#include <algorithm>

namespace WUI
{

Window::Window()
	: controls()
#ifdef _WIN32
	, hWnd(0),
	backgroundBrush(0),
	captionFont(0)
#endif
{
#ifdef _WIN32
	MakePrimitives();
#endif
}

Window::~Window()
{
#ifdef _WIN32
	DestroyPrimitives();
#endif
}

void Window::AddControl(IControl &control, const Rect &position)
{
	if (std::find(controls.begin(), controls.end(), &control) == controls.end())
	{
		control.SetPosition(position);
		control.SetParent(this);
		controls.emplace_back(&control);
	}
}

void Window::RemoveControl(IControl &control)
{
	auto exist = std::find(controls.begin(), controls.end(), &control);
	if (exist != controls.end())
	{
		(*exist)->SetParent(nullptr);
		controls.erase(exist);
	}
}

void Window::ReceiveEvent(Event &ev)
{

}

void Window::UpdateTheme()
{
#ifdef _WIN32
	DestroyPrimitives();
	MakePrimitives();
#endif
}

/// Windows specified code
#ifdef _WIN32

void Window::Show()
{
	ShowWindow(hWnd, SW_SHOW);
}

void Window::Hide()
{
	ShowWindow(hWnd, SW_HIDE);
}

bool Window::Init(WindowType type, const Rect &position, const std::string &caption)
{
	WNDCLASSEXW wcex = { 0 };

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = Window::WndProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = sizeof(this);
	wcex.hInstance = GetModuleHandle(NULL);
	wcex.hbrBackground = backgroundBrush;
	wcex.lpszClassName = L"WUI Window";

	RegisterClassExW(&wcex);

	hWnd = CreateWindow(wcex.lpszClassName, L"", WS_POPUP,
		position.left, position.top, position.right, position.bottom, nullptr, nullptr, GetModuleHandle(NULL), this);

	if (!hWnd)
	{
		return false;
	}

	ShowWindow(hWnd, SW_SHOW);
	UpdateWindow(hWnd);

	return true;
}

LRESULT CALLBACK Window::WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	static Window *wnd = nullptr;
	switch (message)
	{
		case WM_CREATE:
		{
			auto *cs = reinterpret_cast<CREATESTRUCT*>(lParam);
			wnd = reinterpret_cast<Window*>(cs->lpCreateParams);
		}
		break;
		case WM_PAINT:
		{
			PAINTSTRUCT ps;
			HDC hdc = BeginPaint(hWnd, &ps);

			Graphic gr{ hdc };
		
			for (auto &control : wnd->controls)
			{
				control->Draw(gr);
			}

			EndPaint(hWnd, &ps);
		}
		break;
		case WM_DESTROY:
			PostQuitMessage(0);
		break;
		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

void Window::MakePrimitives()
{
	backgroundBrush = CreateSolidBrush(ThemeColor(ThemeValue::Window_Background));
	captionFont = CreateFont(18, 0, 0, 0, FW_DONTCARE, FALSE, FALSE, FALSE, ANSI_CHARSET,
		OUT_TT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY,
		DEFAULT_PITCH | FF_DONTCARE, L"Segoe UI");
}

void Window::DestroyPrimitives()
{
	DeleteObject(backgroundBrush);
	DeleteObject(captionFont);
}

#endif

}
