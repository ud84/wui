// MainFrame.cpp : Defines the main frame impl
//

#include <wui/theme/theme.hpp>
#include <wui/locale/locale.hpp>

#include <MainFrame/MainFrame.h>

#include <Resource.h>

#ifndef _WIN32
#include <iostream>
#endif

MainFrame::MainFrame()
    : window(),
    mainSheet(), windowSheet(), buttonSheet(), inputSheet(), listSheet(), menuSheet(), panelSheet(),
    runned(false)
{
}

void MainFrame::Run()
{
    if (runned)
    {
        return;
    }
    runned = true;

    window = std::shared_ptr<wui::window>(new wui::window());

    mainSheet   = std::shared_ptr<wui::button>(new wui::button(wui::locale("main_frame", "main_sheet"),   std::bind(&MainFrame::ShowMain, this),   wui::button_view::sheet));
    windowSheet = std::shared_ptr<wui::button>(new wui::button(wui::locale("main_frame", "window_sheet"), std::bind(&MainFrame::ShowWindow, this), wui::button_view::sheet));
    buttonSheet = std::shared_ptr<wui::button>(new wui::button(wui::locale("main_frame", "button_sheet"), std::bind(&MainFrame::ShowButton, this), wui::button_view::sheet));
    inputSheet  = std::shared_ptr<wui::button>(new wui::button(wui::locale("main_frame", "input_sheet"),  std::bind(&MainFrame::ShowInput, this),  wui::button_view::sheet));
    listSheet   = std::shared_ptr<wui::button>(new wui::button(wui::locale("main_frame", "list_sheet"),   std::bind(&MainFrame::ShowList, this),   wui::button_view::sheet));
    menuSheet   = std::shared_ptr<wui::button>(new wui::button(wui::locale("main_frame", "menu_sheet"),   std::bind(&MainFrame::ShowMenu, this),   wui::button_view::sheet));
    panelSheet  = std::shared_ptr<wui::button>(new wui::button(wui::locale("main_frame", "panel_sheet"),  std::bind(&MainFrame::ShowPanel, this),  wui::button_view::sheet));

    const auto SHEET_WIDTH = (WND_WIDTH - 20 - 10 * 7) / 7;

    window->add_control(mainSheet,   { 10, 30, SHEET_WIDTH, 55 });
    window->add_control(windowSheet, { 10 * 2 + SHEET_WIDTH, 30, 10 * 2 + SHEET_WIDTH * 2, 55 });
    window->add_control(buttonSheet, { 10 * 3 + SHEET_WIDTH * 2, 30, 10 * 3 + SHEET_WIDTH * 3, 55 });
    window->add_control(inputSheet,  { 10 * 4 + SHEET_WIDTH * 3, 30, 10 * 4 + SHEET_WIDTH * 3, 55 });
    window->add_control(listSheet,   { 10 * 5 + SHEET_WIDTH * 4, 30, 10 * 5 + SHEET_WIDTH * 4, 55 });
    window->add_control(menuSheet,   { 10 * 6 + SHEET_WIDTH * 5, 30, 10 * 6 + SHEET_WIDTH * 5, 55 });
    window->add_control(panelSheet,  { 10 * 7 + SHEET_WIDTH * 6, 30, 10 * 7 + SHEET_WIDTH * 6, 55 });

    ShowMain();

    window->set_control_callback([&](wui::window_control control, std::string &tooltip_text, bool continue_) {
        if (control != wui::window_control::theme)
        {
            return;
        }

        auto theme_name = wui::get_default_theme()->get_name();

        if (theme_name == "dark")
        {
            tooltip_text = wui::locale("window", "dark_theme");
#ifdef _WIN32
            wui::set_default_theme_from_resource("light", TXT_LIGHT_THEME, "JSONS");
#else
            
            if (!wui::set_default_theme_from_file("light", light_theme_json_file))
            {
                std::cerr << "Error opening theme json: " << light_theme_json_file << std::endl;
            }
#endif
        }
        else if (theme_name == "light")
        {
            tooltip_text = wui::locale("window", "light_theme");
#ifdef _WIN32
            wui::set_default_theme_from_resource("dark", TXT_DARK_THEME, "JSONS");
#elif __linux__
            if (!wui::set_default_theme_from_file("dark", dark_theme_json_file))
            {
                std::cerr << "Error opening theme json: " << dark_theme_json_file << std::endl;
            }
#endif
        }

        window->update_theme();
    });

    window->init(wui::locale("main_frame", "caption"), { -1, -1, WND_WIDTH, WND_HEIGHT },
        static_cast<wui::window_style>(static_cast<uint32_t>(wui::window_style::frame) |
        static_cast<uint32_t>(wui::window_style::switch_theme_button) |
        static_cast<uint32_t>(wui::window_style::border_all)), [this]() {
            mainSheet.reset();
            windowSheet.reset();
            buttonSheet.reset();
            inputSheet.reset();
            listSheet.reset();
            menuSheet.reset();
            panelSheet.reset();
            //window.reset(); // todo!

#ifdef _WIN32
            PostQuitMessage(IDCANCEL);
#else
            runned = false;
#endif
    });
}

void MainFrame::ShowMain()
{
    mainSheet->turn(true);
    windowSheet->turn(false);
    buttonSheet->turn(false);
    inputSheet->turn(false);
    listSheet->turn(false);
    menuSheet->turn(false);
    panelSheet->turn(false);
}

void MainFrame::ShowWindow()
{
    mainSheet->turn(false);
    windowSheet->turn(true);
    buttonSheet->turn(false);
    inputSheet->turn(false);
    listSheet->turn(false);
    menuSheet->turn(false);
    panelSheet->turn(false);
}

void MainFrame::ShowButton()
{
    mainSheet->turn(false);
    windowSheet->turn(false);
    buttonSheet->turn(true);
    inputSheet->turn(false);
    listSheet->turn(false);
    menuSheet->turn(false);
    panelSheet->turn(false);
}

void MainFrame::ShowInput()
{
    mainSheet->turn(false);
    windowSheet->turn(false);
    buttonSheet->turn(false);
    inputSheet->turn(true);
    listSheet->turn(false);
    menuSheet->turn(false);
    panelSheet->turn(false);
}

void MainFrame::ShowList()
{
    mainSheet->turn(false);
    windowSheet->turn(false);
    buttonSheet->turn(false);
    inputSheet->turn(false);
    listSheet->turn(true);
    menuSheet->turn(false);
    panelSheet->turn(false);
}

void MainFrame::ShowMenu()
{
    mainSheet->turn(false);
    windowSheet->turn(false);
    buttonSheet->turn(false);
    inputSheet->turn(false);
    listSheet->turn(false);
    menuSheet->turn(true);
    panelSheet->turn(false);
}

void MainFrame::ShowPanel()
{
    mainSheet->turn(false);
    windowSheet->turn(false);
    buttonSheet->turn(false);
    inputSheet->turn(false);
    listSheet->turn(false);
    menuSheet->turn(false);
    panelSheet->turn(true);    
}

bool MainFrame::Runned() const
{
    return runned;
}
