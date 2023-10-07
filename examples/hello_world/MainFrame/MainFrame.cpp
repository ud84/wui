//
// Copyright (c) 2023 Anton Golovkov (udattsk at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/ud84/wui
//

#include <wui/config/config.hpp>
#include <wui/theme/theme.hpp>
#include <wui/locale/locale.hpp>
#include <wui/system/tools.hpp>

#include <MainFrame/MainFrame.h>

#include <Resource.h>

#ifndef _WIN32
#include <iostream>
#endif

MainFrame::MainFrame()
    : window(new wui::window()),
    
    logoImage(new wui::image(IMG_LOGO)),
    whatsYourNameText(new wui::text(wui::locale("main_frame", "whats_your_name_text"), wui::text_alignment::center, "h1_text")),
    userNameInput(new wui::input(wui::config::get_string("User", "Name", ""))),
    okButton(new wui::button(wui::locale("main_frame", "ok_button"), [this](){
        wui::config::set_string("User", "Name", userNameInput->text());
        messageBox->show(wui::locale("main_frame", "hello_text") + userNameInput->text(),
        wui::locale("main_frame", "ok_message_caption"), wui::message_icon::information, wui::message_button::ok, [this](wui::message_result) {
            runned = false; window->destroy(); }); })),
    messageBox(new wui::message(window)),

    runned(false)
{
    window->subscribe(std::bind(&MainFrame::ReceiveEvents,
        this,
        std::placeholders::_1),
        static_cast<wui::event_type>(static_cast<int32_t>(wui::event_type::internal) |
            static_cast<int32_t>(wui::event_type::system) |
            static_cast<int32_t>(wui::event_type::keyboard)));

    window->add_control(logoImage,         { 0 });
    window->add_control(whatsYourNameText, { 0 });
    window->add_control(userNameInput,     { 0 });
    window->add_control(okButton,          { 0 });

    window->set_default_push_control(okButton);

    window->set_min_size(WND_WIDTH - 1, WND_HEIGHT - 1);
}

void MainFrame::Run()
{
    if (runned)
    {
        return;
    }
    runned = true;

    UpdateControlsPosition();

    window->set_control_callback([&](wui::window_control control, std::string &tooltip_text, bool &continue_) {
        switch (control)
        {
            case wui::window_control::theme:
            {
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
            }
            break;
            case wui::window_control::close:
                if (runned)
                {
                    continue_ = false;
                    messageBox->show(wui::locale("main_frame", "confirm_close_text"),
                        wui::locale("main_frame", "cross_message_caption"), wui::message_icon::information, wui::message_button::yes_no,
                        [this, &continue_](wui::message_result r) {
                            continue_ = (r == wui::message_result::yes);
                            runned = !continue_;
                        });
                }
            break;
        }
    });

    auto width = wui::config::get_int("MainFrame", "Width", WND_WIDTH);
    auto height = wui::config::get_int("MainFrame", "Height", WND_HEIGHT);

    window->init(wui::locale("main_frame", "caption"), { -1, -1, width, height },
        static_cast<wui::window_style>(static_cast<uint32_t>(wui::window_style::frame) |
        static_cast<uint32_t>(wui::window_style::switch_theme_button) |
        static_cast<uint32_t>(wui::window_style::border_all)), [this]() {
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
                    wui::config::set_int("MainFrame", "Width", ev.internal_event_.x);
                    wui::config::set_int("MainFrame", "Height", ev.internal_event_.y);
                }
                UpdateControlsPosition();
            break;
            case wui::internal_event_type::window_expanded:
            case wui::internal_event_type::window_normalized:
                UpdateControlsPosition();
            break;
            case wui::internal_event_type::window_minimized:
            break;
        }
    }
}

void MainFrame::UpdateControlsPosition()
{
    const auto width = window->position().width(), height = window->position().height();

    const int32_t top = 40, element_height = 40, space = 30;

    wui::rect pos = { space, top, width - space, top + element_height };
    whatsYourNameText->set_position(pos);
    wui::line_up_top_bottom(pos, element_height, space);
    userNameInput->set_position(pos);
    wui::line_up_top_bottom(pos, element_height * 2, space);
    
    int32_t center = width / 2;

    pos.left = center - element_height, pos.right = center + element_height;

    logoImage->set_position(pos);

    okButton->set_position({center - 80,
        height - element_height - space,
        center + 80,
        height - space
    });
}

bool MainFrame::Runned() const
{
    return runned;
}
