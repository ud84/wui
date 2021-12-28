// wui.cpp : Defines the entry point for the application.
//

#include <wui/theme/theme.hpp>
#include <wui/window/window.hpp>
#include <wui/control/button.hpp>
#include <wui/control/image.hpp>

#ifdef _WIN32
#include <Resource.h>
#include <gdiplus.h>
#endif

struct PluggedWindow
{
	std::shared_ptr<wui::Window> &parentWindow;

	std::shared_ptr<wui::Window> window;
	std::shared_ptr<wui::Button> plugButton, unplugButton;

	void Plug()
	{
		window->Destroy();

		parentWindow->AddControl(window, wui::Rect{ 50, 50, 250, 250 });

		window->AddControl(unplugButton, wui::Rect{ 10, 10, 110, 35 });
		window->AddControl(plugButton, wui::Rect{ 10, 55, 110, 80 });

		plugButton->Disable();
		unplugButton->Enable();
	}

	void Unplug()
	{
		parentWindow->RemoveControl(window);
		window->Init(wui::WindowType::Dialog, wui::Rect{ 50, 50, 250, 250 }, L"Child window unplugged!", []() {});

		plugButton->Enable();
		unplugButton->Disable();
	}

	PluggedWindow(std::shared_ptr<wui::Window> &parentWindow_)
		: parentWindow(parentWindow_),
		window(new wui::Window()),
		plugButton(new wui::Button(L"Plug Window", std::bind(&PluggedWindow::Plug, this))),
		unplugButton(new wui::Button(L"Unplug Window", std::bind(&PluggedWindow::Unplug, this)))
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

	wui::SetDefaultTheme(wui::Theme::Dark);

	std::shared_ptr<wui::Window> window(new wui::Window());

	PluggedWindow pluggedWindow(window);
	pluggedWindow.Plug();

	std::shared_ptr<wui::Window> dialog(new wui::Window());

	std::shared_ptr<wui::Button> okButton(new wui::Button(L"OK", [window, &dialog]()
	{
		window->Block();

		std::shared_ptr<wui::Button> dialogButton(new wui::Button(L"Close", [&dialog]() { dialog->Destroy(); }));
		dialog->AddControl(dialogButton, wui::Rect{ 10, 200, 100, 235 });

		dialog->Init(wui::WindowType::Dialog, wui::Rect{ 50, 50, 250, 250 }, L"Modal dialog", [window, &dialog]() { window->Unlock(); /*dialog.reset();*/ });
	}));

	auto redButtonTheme = wui::MakeCustomTheme();
	redButtonTheme->SetColor(wui::ThemeValue::Button_Calm, wui::MakeColor(205, 15, 20));
	redButtonTheme->SetColor(wui::ThemeValue::Button_Active, wui::MakeColor(235, 15, 20));
	redButtonTheme->SetColor(wui::ThemeValue::Button_Border, wui::MakeColor(200, 215, 200));
	redButtonTheme->SetColor(wui::ThemeValue::Button_FocusedBorder, wui::MakeColor(20, 215, 20));
	redButtonTheme->SetColor(wui::ThemeValue::Button_Text, wui::MakeColor(190, 205, 190));
	redButtonTheme->SetColor(wui::ThemeValue::Button_Disabled, wui::MakeColor(180, 190, 180));
	redButtonTheme->SetDimension(wui::ThemeValue::Button_Round, 5);
	redButtonTheme->SetString(wui::ThemeValue::Images_Path, L"IMAGES_DARK");
	std::shared_ptr<wui::Button> cancelButton(new wui::Button(L"Cancel", [window]() { window->Destroy(); }, wui::ButtonView::ImageRightText, IDB_ACCOUNT, 24, redButtonTheme));

	std::shared_ptr<wui::Button> darkThemeButton(new wui::Button(L"Set the dark theme", [window, pluggedWindow, dialog]() { window->ShowTitle(); wui::SetDefaultTheme(wui::Theme::Dark); window->UpdateTheme(); pluggedWindow.window->UpdateTheme(); dialog->UpdateTheme(); }));
	window->AddControl(darkThemeButton, wui::Rect{ 140, 350, 150, 375 });
	
	std::shared_ptr<wui::Button> whiteThemeButton(new wui::Button(L"Set the white theme", [window, pluggedWindow, dialog]() { wui::SetDefaultTheme(wui::Theme::White); window->UpdateTheme(); pluggedWindow.window->UpdateTheme(); dialog->UpdateTheme(); }));
	window->AddControl(whiteThemeButton, wui::Rect{ 270, 350, 380, 375 });

	window->AddControl(okButton, wui::Rect{ 240, 450, 350, 480 });
	window->AddControl(cancelButton, wui::Rect{ 370, 450, 480, 480 });

	std::shared_ptr<wui::Image> accountImage(new wui::Image(IDB_ACCOUNT));
	window->AddControl(accountImage, wui::Rect{ 250, 100, 314, 164 });

	/*auto fileImageTheme = wui::MakeCustomTheme();
	fileImageTheme->SetString(wui::ThemeValue::Images_Path, L"F:\\Docs");
	std::shared_ptr<wui::Image> fileImage(new wui::Image(L"kavareiya_1.png", fileImageTheme));
	window->AddControl(fileImage, wui::Rect{ 180, 200, 344, 344 });*/

	window->Init(wui::WindowType::Frame, wui::Rect{ 100, 100, 500, 500 }, L"Welcome to WUI!", []() { PostQuitMessage(IDCANCEL); });
	
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
