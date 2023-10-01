//
// Copyright (c) 2023 Anton Golovkov (udattsk at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/ud84/wui
//

#pragma once

#include <wui/window/window.hpp>
#include <wui/control/button.hpp>

#include <MainSheet/MainSheet.h>

enum class Sheet
{
    Main,
    Window,
    Button,
    Input,
    List,
    Menu,
    Panel
};

class MainFrame
{
public:
    MainFrame();

    void Run();

    bool Runned() const;

private:
    static const int32_t WND_WIDTH = 800, WND_HEIGHT = 600;

    std::shared_ptr<wui::window> window;

    std::shared_ptr<wui::button> mainSheet, windowSheet, buttonSheet, inputSheet, listSheet, menuSheet, panelSheet;
    std::shared_ptr<wui::button> accountButton, menuButton;

    bool runned;

    Sheet sheet;

    MainSheet mainSheetImpl;

    void ReceiveEvents(const wui::event &ev);

    void UpdateSheets();
};
