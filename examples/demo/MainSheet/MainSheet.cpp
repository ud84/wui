//
// Copyright (c) 2023 Anton Golovkov (udattsk at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/ud84/wui
//

#include <MainSheet/MainSheet.h>

#include <Resource.h>

#include <wui/common/about.hpp>
#include <wui/locale/locale.hpp>
#include <wui/system/tools.hpp>

MainSheet::MainSheet()
    : parentWindow_(),
    logoImage(new wui::image(IMG_LOGO)),
    welcomeText(new wui::text(wui::locale("main_sheet", "main_title"), wui::text_alignment::left, "h1_text")),
    mainSiteAnchor(new wui::button(wui::about::web, [](){ wui::open_uri(wui::about::web); }, wui::button_view::anchor))
{
}

void MainSheet::Run(std::weak_ptr<wui::window> parentWindow__)
{
    parentWindow_ = parentWindow__;

    auto parentWindow = parentWindow_.lock();
    if (parentWindow)
    {
        auto parentPos = parentWindow->position();
        auto width = parentPos.width(), height = parentPos.height();

        parentWindow->add_control(logoImage, { 0 });
        parentWindow->add_control(welcomeText, { 0 });
        parentWindow->add_control(mainSiteAnchor, { 0 });

        UpdateSize(width, height);
    }

    logoImage->update_theme();
    welcomeText->update_theme();
    mainSiteAnchor->update_theme();
}

void MainSheet::End()
{
    auto parentWindow = parentWindow_.lock();
    if (parentWindow)
    {
        parentWindow->remove_control(logoImage);
        parentWindow->remove_control(welcomeText);
        parentWindow->remove_control(mainSiteAnchor);
    }
}

void MainSheet::UpdateSize(int32_t width, int32_t height)
{
    logoImage->set_position({ 10, 100, 60, 150 });
    welcomeText->set_position({ 70, 122, width - 10, 150 });
    mainSiteAnchor->set_position({ 10, height - 40, width - 10, height - 20 });
}