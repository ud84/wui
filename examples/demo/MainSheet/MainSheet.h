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
#include <wui/control/button.hpp>


class MainSheet
{
public:
    MainSheet();

    void Run(std::weak_ptr<wui::window> parentWindow_);
    void End();

    void UpdateSize(int32_t width, int32_t height);

private:
    std::weak_ptr<wui::window> parentWindow_;
    
    std::shared_ptr<wui::image> logoImage;
    std::shared_ptr<wui::text> welcomeText;
    std::shared_ptr<wui::button> mainSiteAnchor;
};
