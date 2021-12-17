#include <WUI/Window/Window.h>

#include <algorithm>

namespace WUI
{

Window::Window()
	: controls()
#ifdef _WIN32
	, hWnd(0)
#endif
{
}

Window::~Window()
{
}

void Window::Init(WindowType type, const Rect &position, const std::string &caption)
{
	MyRegisterClass();

	hWnd = CreateWindowW(L"WUI Window", L"", WS_POPUP,
		position.left, position.top, position.right, position.bottom, nullptr, nullptr, GetModuleHandle(NULL), nullptr);

	if (!hWnd)
	{
		//return FALSE;
	}

	ShowWindow(hWnd, SW_SHOW);
	UpdateWindow(hWnd);
}

void Window::AddControl(IControl &control, const Rect &position)
{
	if (std::find(controls.begin(), controls.end(), &control) == controls.end())
	{
		controls.emplace_back(&control);
	}
}

void Window::RemoveControl(IControl &control)
{
	auto exist = std::find(controls.begin(), controls.end(), &control);
	if (exist != controls.end())
	{
		controls.erase(exist);
	}
}

void Window::ReceiveEvent(Event &ev)
{

}

void Window::Show()
{
	ShowWindow(hWnd, SW_SHOW);
}

void Window::Hide()
{
	ShowWindow(hWnd, SW_HIDE);
}

/// Windows specified code
void Window::MyRegisterClass()
{
	WNDCLASSEXW wcex = { 0 };

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = Window::WndProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = GetModuleHandle(NULL);
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wcex.lpszClassName = L"WUI Window";
	
	RegisterClassExW(&wcex);
}

LRESULT CALLBACK Window::WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_COMMAND:
	{
		int wmId = LOWORD(wParam);
		// Parse the menu selections:
		switch (wmId)
		{
		//case IDM_ABOUT:
			//DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
			break;
		//case IDM_EXIT:
			//DestroyWindow(hWnd);
			break;
		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
		}
	}
	break;
	case WM_PAINT:
	{
		PAINTSTRUCT ps;
		HDC hdc = BeginPaint(hWnd, &ps);
		// TODO: Add any drawing code that uses hdc here...
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

}
