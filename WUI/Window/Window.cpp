#include <WUI/Window/Window.h>

#include <WUI/Graphic/Graphic.h>

#include <WUI/Theme/Theme.h>

#include <algorithm>

#ifdef _WIN32
#include <windowsx.h>
#endif

namespace WUI
{

Window::Window()
	: controls(), activeControls()
#ifdef _WIN32
	, hWnd(0),
	backgroundBrush(0),
	font(0),
	xClick(0), yClick(0)
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

void Window::Redraw(const Rect &position)
{
#ifdef _WIN32
	RECT invalidatingRect = { position.left, position.top, position.right, position.bottom };
	InvalidateRect(hWnd, &invalidatingRect, FALSE);
#endif
}

void Window::UpdateTheme()
{
#ifdef _WIN32
	DestroyPrimitives();
	MakePrimitives();
#endif
}

void Window::SendMouseEvent(const MouseEvent &ev)
{
	for (auto &control : controls)
	{
		if (control->GetPosition().In(ev.x, ev.y))
		{
			if (std::find(activeControls.begin(), activeControls.end(), control) != activeControls.end())
			{
				control->ReceiveEvent({ EventType::Mouse, ev });
			}
			else
			{
				activeControls.emplace_back(control);

				MouseEvent me{ MouseEventType::Enter, 0, 0 };
				control->ReceiveEvent({ EventType::Mouse, me });
			}

			break;
		}
		else
		{
			auto activeControl = std::find(activeControls.begin(), activeControls.end(), control);
			if (activeControl != activeControls.end())
			{
				activeControls.erase(activeControl);

				MouseEvent me{ MouseEventType::Leave, 0, 0 };
				control->ReceiveEvent({ EventType::Mouse, me });
			}
		}
	}
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
	wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);

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

			SelectObject(hdc, wnd->font);
		
			for (auto &control : wnd->controls)
			{
				control->Draw(gr);
			}

			EndPaint(hWnd, &ps);
		}
		break;
		case WM_MOUSEMOVE:
			if (GetCapture() == wnd->hWnd)
			{
				RECT rcWindow;
				GetWindowRect(wnd->hWnd, &rcWindow);

				int16_t xMouse = GET_X_LPARAM(lParam);
				int16_t yMouse = GET_Y_LPARAM(lParam);

				int32_t xWindow = rcWindow.left + xMouse - wnd->xClick;
				int32_t yWindow = rcWindow.top + yMouse - wnd->yClick;

				SetWindowPos(wnd->hWnd, NULL, xWindow, yWindow, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
			}
			wnd->SendMouseEvent({ MouseEventType::Move, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) });
		break;
		case WM_LBUTTONDOWN:
			SetCapture(wnd->hWnd);
			wnd->xClick = GET_X_LPARAM(lParam);
			wnd->yClick = GET_Y_LPARAM(lParam);

			wnd->SendMouseEvent({ MouseEventType::LeftDown, wnd->xClick, wnd->yClick });
		break;
		case WM_LBUTTONUP:
			ReleaseCapture();

			wnd->SendMouseEvent({ MouseEventType::LeftUp, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) });
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
	font = CreateFont(18, 0, 0, 0, FW_DONTCARE, FALSE, FALSE, FALSE, ANSI_CHARSET,
		OUT_TT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY,
		DEFAULT_PITCH | FF_DONTCARE, L"Segoe UI");
}

void Window::DestroyPrimitives()
{
	DeleteObject(backgroundBrush);
	DeleteObject(font);
}

#endif

}
