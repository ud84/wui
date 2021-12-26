// wui.cpp : Defines the entry point for the application.
//

#include "wui.h"

#include <WUI/Theme/Theme.h>
#include <WUI/Window/Window.h>
#include <WUI/Control/Button.h>
#include <WUI/Control/Image.h>

#ifdef _WIN32
#include <gdiplus.h>
#endif

struct PluggedWindow
{
	std::shared_ptr<WUI::Window> &parentWindow;

	std::shared_ptr<WUI::Window> window;
	std::shared_ptr<WUI::Button> plugButton, unplugButton;

	void Plug()
	{
		window->Destroy();

		parentWindow->AddControl(window, WUI::Rect{ 50, 50, 250, 250 });

		window->AddControl(unplugButton, WUI::Rect{ 10, 10, 110, 35 });
		window->AddControl(plugButton, WUI::Rect{ 10, 55, 110, 80 });

		plugButton->Disable();
		unplugButton->Enable();
	}

	void Unplug()
	{
		parentWindow->RemoveControl(window);
		window->Init(WUI::WindowType::Dialog, WUI::Rect{ 50, 50, 250, 250 }, L"Child window unplugged!", []() {});

		plugButton->Enable();
		unplugButton->Disable();
	}

	PluggedWindow(std::shared_ptr<WUI::Window> &parentWindow_)
		: parentWindow(parentWindow_),
		window(new WUI::Window()),
		plugButton(new WUI::Button(L"Plug Window", std::bind(&PluggedWindow::Plug, this))),
		unplugButton(new WUI::Button(L"Unplug Window", std::bind(&PluggedWindow::Unplug, this)))
	{}
};


int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

#ifdef _WIN32
	Gdiplus::GdiplusStartupInput gdiplusStartupInput;
	ULONG_PTR gdiplusToken;
	Gdiplus::GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);
#endif

	WUI::SetDefaultTheme(WUI::Theme::Dark);

	std::shared_ptr<WUI::Window> window(new WUI::Window());

	PluggedWindow pluggedWindow(window);
	pluggedWindow.Plug();

	std::shared_ptr<WUI::Window> dialog(new WUI::Window());

	std::shared_ptr<WUI::Button> okButton(new WUI::Button(L"OK", [window, &dialog]() 
	{ 
		window->Block();

		std::shared_ptr<WUI::Button> dialogButton(new WUI::Button(L"Close", [&dialog]() { dialog->Destroy(); }));
		dialog->AddControl(dialogButton, WUI::Rect{ 10, 200, 100, 235 });

		dialog->Init(WUI::WindowType::Dialog, WUI::Rect{ 50, 50, 250, 250 }, L"Modal dialog", [window, &dialog]() { window->Unlock(); /*dialog.reset();*/ });
	}));

	auto redButtonTheme = WUI::MakeCustomTheme();
	redButtonTheme->SetColor(WUI::ThemeValue::Button_Calm, WUI::MakeColor(205, 15, 20));
	redButtonTheme->SetColor(WUI::ThemeValue::Button_Active, WUI::MakeColor(235, 15, 20));
	redButtonTheme->SetColor(WUI::ThemeValue::Button_Border, WUI::MakeColor(200, 215, 200));
	redButtonTheme->SetColor(WUI::ThemeValue::Button_FocusedBorder, WUI::MakeColor(20, 215, 20));
	redButtonTheme->SetColor(WUI::ThemeValue::Button_Text, WUI::MakeColor(190, 205, 190));
	redButtonTheme->SetColor(WUI::ThemeValue::Button_Disabled, WUI::MakeColor(180, 190, 180));
	redButtonTheme->SetDimension(WUI::ThemeValue::Button_Round, 5);
	std::shared_ptr<WUI::Button> cancelButton(new WUI::Button(L"Cancel", [window]() { window->Destroy(); }, redButtonTheme));

	std::shared_ptr<WUI::Button> darkThemeButton(new WUI::Button(L"Set the dark theme", [window, pluggedWindow, dialog]() { WUI::SetDefaultTheme(WUI::Theme::Dark); window->UpdateTheme(); pluggedWindow.window->UpdateTheme(); dialog->UpdateTheme(); }));
	window->AddControl(darkThemeButton, WUI::Rect{ 140, 350, 150, 375 });
	
	std::shared_ptr<WUI::Button> whiteThemeButton(new WUI::Button(L"Set the white theme", [window, pluggedWindow, dialog]() { WUI::SetDefaultTheme(WUI::Theme::White); window->UpdateTheme(); pluggedWindow.window->UpdateTheme(); dialog->UpdateTheme(); }));
	window->AddControl(whiteThemeButton, WUI::Rect{ 270, 350, 380, 375 });

	window->AddControl(okButton, WUI::Rect{ 240, 450, 350, 475 });
	window->AddControl(cancelButton, WUI::Rect{ 370, 450, 480, 475 });

	std::shared_ptr<WUI::Image> accountImage(new WUI::Image(IDB_ACCOUNT));
	window->AddControl(accountImage, WUI::Rect{ 200, 100, 264, 164 });

	/*auto fileImageTheme = WUI::MakeCustomTheme();
	fileImageTheme->SetString(WUI::ThemeValue::Images_Path, L"F:\\Docs");
	std::shared_ptr<WUI::Image> fileImage(new WUI::Image(L"kavareiya_1.png", fileImageTheme));
	window->AddControl(fileImage, WUI::Rect{ 200, 200, 264, 264 });*/

	window->Init(WUI::WindowType::Frame, WUI::Rect{ 100, 100, 500, 500 }, L"Welcome to WUI!", []() { PostQuitMessage(IDCANCEL); });
	
#ifdef _WIN32
	// Main message loop:
	MSG msg;
	while (GetMessage(&msg, nullptr, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	//Gdiplus::GdiplusShutdown(gdiplusToken);
	return (int) msg.wParam;
#endif
	return 0;
}
