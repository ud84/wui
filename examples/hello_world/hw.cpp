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
#include <wui/theme/theme_selector.hpp>

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
    auto ok = wui::config::use_ini_file("hello_world.ini");
    if (!ok)
    {
        std::cerr << wui::config::get_error().str() << std::endl;
        return -1;
    }
#endif

    wui::error err;

    wui::set_app_locales({
        { wui::locale_type::eng, "English", "res/en_locale.json", TXT_LOCALE_EN },
        { wui::locale_type::rus, "Русский", "res/ru_locale.json", TXT_LOCALE_RU },
    });

    auto current_locale = static_cast<wui::locale_type>(wui::config::get_int("User", "Locale", 
        static_cast<int32_t>(wui::get_default_system_locale())));

    wui::set_current_app_locale(current_locale);

    wui::set_locale_from_type(current_locale, err);
    if (!err.is_ok())
    {
        std::cerr << err.str() << std::endl;
        return -1;
    }

    wui::set_app_themes({
        { "dark",  "res/dark.json",  TXT_DARK_THEME },
        { "light", "res/light.json", TXT_LIGHT_THEME }
    });

    auto current_theme = wui::config::get_string("User", "Theme", "dark");
    wui::set_default_theme("dark");
    wui::set_current_app_theme(current_theme);

    wui::set_default_theme_from_name(current_theme, err);
    if (!err.is_ok())
    {
        std::cerr << err.str() << std::endl;
        return -1;
    }

    MainFrame mainFrame;
    mainFrame.Run();

#ifdef _WIN32
    // Main message loop
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
