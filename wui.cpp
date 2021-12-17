// wui.cpp : Defines the entry point for the application.
//

#include "wui.h"

#include <WUI/Window/Window.h>
#include <WUI/Control/Button.h>

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

	WUI::Window window;

	WUI::Button okButton("OK", []() { MessageBox(NULL, L"OK was clicked!", L"Yes", MB_ICONEXCLAMATION); });

	window.AddControl(okButton, WUI::Rect(10, 10, 110, 35));

	window.Init(WUI::WindowType::Dialog, WUI::Rect(100, 100, 500, 500), "test");
	
	// Main message loop:
	MSG msg;
	while (GetMessage(&msg, nullptr, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	
	return (int) msg.wParam;
}
