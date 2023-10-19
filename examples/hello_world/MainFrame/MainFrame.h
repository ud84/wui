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

#include <wui/control/image.hpp>
#include <wui/control/text.hpp>
#include <wui/control/input.hpp>
#include <wui/control/button.hpp>
#include <wui/control/message.hpp>

class MainFrame
{
public:
    MainFrame();

    void Run();

private:
    static const int32_t WND_WIDTH = 400, WND_HEIGHT = 400;

    std::shared_ptr<wui::window> window;

    std::shared_ptr<wui::image> logoImage;
    std::shared_ptr<wui::text> whatsYourNameText;
    std::shared_ptr<wui::input> userNameInput;
    std::shared_ptr<wui::button> okButton;
    std::shared_ptr<wui::message> messageBox;

    bool runned;

    void ReceiveEvents(const wui::event &ev);

    void UpdateControlsPosition();
};
