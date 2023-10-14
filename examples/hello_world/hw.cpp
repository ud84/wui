//
// Copyright (c) 2021-2023 Anton Golovkov (udattsk at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/ud84/wui
//

#include <wui/config/config.hpp>
#include <wui/theme/theme.hpp>

#include <wui/locale/locale.hpp>
#include <wui/locale/locale_selector.hpp>

#include <MainFrame/MainFrame.h>

#include <Resource.h>

#ifdef _WIN32
#include <gdiplus.h>
#endif

#include <iostream>

#ifdef _WIN32
int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
    _In_opt_ HINSTANCE hPrevInstance,
    _In_ LPWSTR    lpCmdLine,
    _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    Gdiplus::GdiplusStartupInput gdiplusStartupInput;
    ULONG_PTR gdiplusToken;
    Gdiplus::GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);
#elif __linux__
int main(int argc, char *argv[])
{
    if (setlocale(LC_ALL, "") == NULL)
    {
        std::cerr << "warning: could not set default locale" << std::endl;
    }

#endif

#ifdef _WIN32
    auto ok = wui::config::use_registry("Software\\wui\\hello_world");
#else
    auto ok = wui::config::use_ini_file(config_ini_file);
    if (!ok)
    {
        std::cerr << "can't open config: " << config_ini_file << std::endl;
        return -1;
    }
#endif

    wui::set_app_locales({
        { wui::locale_type::eng, "English", "res/en_locale.json", TXT_LOCALE_EN },
        { wui::locale_type::rus, "Русский", "res/ru_locale.json", TXT_LOCALE_RU },
    });

    auto current_locale = static_cast<wui::locale_type>(wui::config::get_int("User", "Locale", static_cast<int32_t>(wui::get_default_system_locale())));
    wui::set_current_app_locale(current_locale);
    wui::set_locale_from_type(current_locale);

	bool darkTheme = wui::config::get_int("User", "Theme", 0) == 0;

#ifdef _WIN32
	ok = wui::set_default_theme_from_resource(darkTheme ? "dark" : "light", darkTheme ? TXT_DARK_THEME : TXT_LIGHT_THEME, "JSONS");
	if (!ok)
	{
		std::cerr << "can't load theme from resource" << std::endl;
		return -1;
	}
#else
	ok = wui::set_default_theme_from_file(darkTheme ? "dark" : "light", darkTheme ? dark_theme_json_file : light_theme_json_file);
	if (!ok)
	{
		std::cerr << "No theme file: " << (darkTheme ? dark_theme_json_file : light_theme_json_file) << " was found or contains an invalid json" << std::endl;
		return -1;
	}
#endif

    MainFrame mainFrame;
    mainFrame.Run();

#ifdef _WIN32
    // Main message loop:
    MSG msg;
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    return (int) msg.wParam;
#elif __linux__
    // Wait for main window
    while (mainFrame.Runned())
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    return 0;
#endif
}
