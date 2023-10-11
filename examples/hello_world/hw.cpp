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
    wui::config::use_registry("Software\\wui\\hello_world");

	int32_t defaultLocale = en_locale_index;
	switch (GetUserDefaultUILanguage()) /// See the: https://www.autoitscript.com/autoit3/docs/appendix/OSLangCodes.htm
	{
		case 1049: defaultLocale = ru_locale_index; break;
		case 1087: defaultLocale = kk_locale_index; break;
	}
	auto localeIndex = wui::config::get_int("User", "Locale", defaultLocale);
	std::string localeName = "en";
	int32_t localeResource = TXT_LOCALE_EN;
	switch (localeIndex)
	{
		case en_locale_index: localeName = "en"; localeResource = TXT_LOCALE_EN; break;
		case ru_locale_index: localeName = "ru"; localeResource = TXT_LOCALE_RU; break;
	}
	bool ok = wui::set_locale_from_resource(localeName, localeResource, "JSONS");
	if (!ok)
	{
		std::cerr << "can't load locale from resource" << std::endl;
		return -1;
	}

	bool darkTheme = wui::config::get_int("User", "Theme", light_theme_index) == 0;
	ok = wui::set_default_theme_from_resource(darkTheme ? "dark" : "light", darkTheme ? TXT_DARK_THEME : TXT_LIGHT_THEME, "JSONS");
	if (!ok)
	{
		std::cerr << "can't load theme from resource" << std::endl;
		return -1;
	}

#else
    auto ok = wui::config::use_ini_file(config_ini_file);
    if (!ok)
    {
        std::cerr << "can't open config: " << config_ini_file << std::endl;
        return -1;
    }

	auto localeIndex = wui::config::get_int("User", "Locale", 2);
	std::string localeName = "en", localeFile = en_locale_json_file;
	switch (localeIndex)
	{
		case 1: localeName = "en"; localeFile = en_locale_json_file; break;
		case 2: localeName = "ru"; localeFile = ru_locale_json_file; break;
	}
	ok = wui::set_locale_from_file(localeName, localeFile);
	if (!ok)
	{
		std::cerr << "No locale file: " << localeFile << " was found or contains an invalid json" << std::endl;
		return -1;
	}

	bool darkTheme = wui::config::get_int("User", "Theme", 1) == 0;
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

    //Gdiplus::GdiplusShutdown(gdiplusToken);
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
