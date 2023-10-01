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
    runned(false),
    sheet(Sheet::Main)
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

    window->subscribe(std::bind(&MainFrame::ReceiveEvents,
        this,
        std::placeholders::_1),
        static_cast<wui::event_type>(static_cast<int32_t>(wui::event_type::internal) |
            static_cast<int32_t>(wui::event_type::system) |
            static_cast<int32_t>(wui::event_type::keyboard)));

    mainSheet   = std::shared_ptr<wui::button>(new wui::button(wui::locale("main_frame", "main_sheet"),   std::bind(&MainFrame::ShowMain, this),   wui::button_view::sheet));
    windowSheet = std::shared_ptr<wui::button>(new wui::button(wui::locale("main_frame", "window_sheet"), std::bind(&MainFrame::ShowWindow, this), wui::button_view::sheet));
    buttonSheet = std::shared_ptr<wui::button>(new wui::button(wui::locale("main_frame", "button_sheet"), std::bind(&MainFrame::ShowButton, this), wui::button_view::sheet));
    inputSheet  = std::shared_ptr<wui::button>(new wui::button(wui::locale("main_frame", "input_sheet"),  std::bind(&MainFrame::ShowInput, this),  wui::button_view::sheet));
    listSheet   = std::shared_ptr<wui::button>(new wui::button(wui::locale("main_frame", "list_sheet"),   std::bind(&MainFrame::ShowList, this),   wui::button_view::sheet));
    menuSheet   = std::shared_ptr<wui::button>(new wui::button(wui::locale("main_frame", "menu_sheet"),   std::bind(&MainFrame::ShowMenu, this),   wui::button_view::sheet));
    panelSheet  = std::shared_ptr<wui::button>(new wui::button(wui::locale("main_frame", "panel_sheet"),  std::bind(&MainFrame::ShowPanel, this),  wui::button_view::sheet));

    window->add_control(mainSheet,   { 0 });
    window->add_control(windowSheet, { 0 });
    window->add_control(buttonSheet, { 0 });
    window->add_control(inputSheet,  { 0 });
    window->add_control(listSheet,   { 0 });
    window->add_control(menuSheet,   { 0 });
    window->add_control(panelSheet,  { 0 });

    UpdateSheets();

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

void MainFrame::ReceiveEvents(const wui::event &ev)
{
    if (ev.type == wui::event_type::internal)
    {
        switch (ev.internal_event_.type)
        {
            case wui::internal_event_type::window_created:
        
            break;
            case wui::internal_event_type::size_changed:
                if (window->state() == wui::window_state::normal &&
                    ev.internal_event_.x > 0 && ev.internal_event_.y > 0)
                {
                    UpdateSheets();
                }
            break;
            case wui::internal_event_type::window_expanded:
                UpdateSheets();
            break;
            case wui::internal_event_type::window_normalized:
                UpdateSheets();
            break;
            case wui::internal_event_type::window_minimized:
            break;
        }
    }
}

void MainFrame::UpdateSheets()
{
    const auto SHEET_WIDTH = (window->position().width() - 20 - 10 * 7) / 7;

    mainSheet  ->set_position({ 10, 30, SHEET_WIDTH, 55 });
    windowSheet->set_position({ 10 * 2 + SHEET_WIDTH, 30, 10 * 2 + SHEET_WIDTH * 2, 55 });
    buttonSheet->set_position({ 10 * 3 + SHEET_WIDTH * 2, 30, 10 * 3 + SHEET_WIDTH * 3, 55 });
    inputSheet ->set_position({ 10 * 4 + SHEET_WIDTH * 3, 30, 10 * 4 + SHEET_WIDTH * 4, 55 });
    listSheet  ->set_position({ 10 * 5 + SHEET_WIDTH * 4, 30, 10 * 5 + SHEET_WIDTH * 5, 55 });
    menuSheet  ->set_position({ 10 * 6 + SHEET_WIDTH * 5, 30, 10 * 6 + SHEET_WIDTH * 6, 55 });
    panelSheet ->set_position({ 10 * 7 + SHEET_WIDTH * 6, 30, 10 * 7 + SHEET_WIDTH * 7, 55 });

    mainSheet->turn(false);
    windowSheet->turn(false);
    buttonSheet->turn(false);
    inputSheet->turn(false);
    listSheet->turn(false);
    menuSheet->turn(false);
    panelSheet->turn(false);

    switch (sheet)
    {
        case Sheet::Main:
            mainSheet->turn(true);
        break;
        case Sheet::Window:
            windowSheet->turn(true);
        break;
        case Sheet::Button:
            buttonSheet->turn(true);
        break;
        case Sheet::Input:
            inputSheet->turn(true);
        break;
        case Sheet::List:
            listSheet->turn(true);
        break;
        case Sheet::Menu:
            menuSheet->turn(true);
        break;
        case Sheet::Panel:
            panelSheet->turn(true);
        break;
    }
}

void MainFrame::ShowMain()
{
    sheet = Sheet::Main;
    UpdateSheets();
}

void MainFrame::ShowWindow()
{
    sheet = Sheet::Window;
    UpdateSheets();
}

void MainFrame::ShowButton()
{
    sheet = Sheet::Button;
    UpdateSheets();
}

void MainFrame::ShowInput()
{
    sheet = Sheet::Input;
    UpdateSheets();
}

void MainFrame::ShowList()
{
    sheet = Sheet::List;
    UpdateSheets();
}

void MainFrame::ShowMenu()
{
    sheet = Sheet::Menu;
    UpdateSheets();
}

void MainFrame::ShowPanel()
{
    sheet = Sheet::Panel;
    UpdateSheets();
}

bool MainFrame::Runned() const
{
    return runned;
}
