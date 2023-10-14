//
// Copyright (c) 2023 Anton Golovkov (udattsk at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/ud84/wui
//

#include <ButtonSheet/ButtonSheet.h>

#include <Resource.h>

#include <wui/common/about.hpp>
#include <wui/locale/locale.hpp>
#include <wui/system/tools.hpp>
#include <wui/system/uri_tools.hpp>

ButtonSheet::ButtonSheet()
    : parentWindow_(),
    buttonsText(new wui::text("", wui::text_alignment::left, "h1_text")),
    simpleButton0(new wui::button(wui::locale("button_sheet", "simple0"), [this]() { buttonsText->set_text("First button pushed"); }, wui::button_view::text)),
    simpleButton1(new wui::button(wui::locale("button_sheet", "simple1"), [this]() { buttonsText->set_text("Second button pushed"); }, wui::button_view::text)),
    imageButton(new wui::button("", [this]() { buttonsText->set_text("Image button pushed"); }, wui::button_view::image, IMG_LOGO, 32)),
    imageRightTextButton(new wui::button(wui::locale("button_sheet", "right_text"), [this]() { buttonsText->set_text("Image with right text button pushed"); }, wui::button_view::image_right_text, IMG_LOGO, 32)),
    imageBottomTextButton(new wui::button(wui::locale("button_sheet", "bottom_text"), [this]() { buttonsText->set_text("Image with bottom text button pushed"); }, wui::button_view::image_bottom_text, IMG_LOGO, 32)),
    switcherButton(new wui::button(wui::locale("button_sheet", "switcher_text"), [this]() { buttonsText->set_text("Switcher button pushed"); }, wui::button_view::switcher)),
    radioButton0(new wui::button(wui::locale("button_sheet", "radio0_text"), [this]() { buttonsText->set_text("Radio first pushed"); radioButton1->turn(!radioButton0->turned()); }, wui::button_view::radio)),
    radioButton1(new wui::button(wui::locale("button_sheet", "radio1_text"), [this]() { buttonsText->set_text("Radio second pushed"); radioButton0->turn(!radioButton1->turned()); }, wui::button_view::radio)),
    anchorButton(new wui::button(wui::locale("button_sheet", "anchor_text"), [this]() { buttonsText->set_text("Anchor pushed"); }, wui::button_view::anchor)),
    sheetButton0(new wui::button(wui::locale("button_sheet", "sheet0_text"), [this]() { buttonsText->set_text("First sheet pushed"); sheetButton0->turn(true); sheetButton1->turn(false); }, wui::button_view::sheet)),
    sheetButton1(new wui::button(wui::locale("button_sheet", "sheet1_text"), [this]() { buttonsText->set_text("Second sheet pushed"); sheetButton0->turn(false); sheetButton1->turn(true); }, wui::button_view::sheet))
{
    radioButton0->turn(true);
    sheetButton0->turn(true);
}

void ButtonSheet::Run(std::weak_ptr<wui::window> parentWindow__)
{
    parentWindow_ = parentWindow__;

    auto parentWindow = parentWindow_.lock();
    if (parentWindow)
    {
        auto parentPos = parentWindow->position();
        auto width = parentPos.width(), height = parentPos.height();

        parentWindow->add_control(buttonsText, { 10, 100, 800, 150 });

        parentWindow->add_control(simpleButton0, { 10, 200, 200, 230 });
        parentWindow->add_control(simpleButton1, { 220, 200, 420, 230 });

        parentWindow->add_control(imageButton, { 10, 250, 10 + 48, 250 + 48 });
        parentWindow->add_control(imageRightTextButton, { 70, 250, 300, 250 + 48 });

        parentWindow->add_control(imageBottomTextButton, { 10, 320, 200, 320 + 80 });

        parentWindow->add_control(switcherButton, { 10, 420, 200, 420 + 30 });

        parentWindow->add_control(radioButton0, { 10, 460, 150, 460 + 30 });
        parentWindow->add_control(radioButton1, { 160, 460, 310, 460 + 30 });

        parentWindow->add_control(anchorButton, { 10, 510, 100, 510 + 30 });

        parentWindow->add_control(sheetButton0, { 10, 550, 100, 550 + 30 });
        parentWindow->add_control(sheetButton1, { 110, 550, 210, 550 + 30 });
    }
    
    sheetButton0->update_theme();
    sheetButton1->update_theme();
    buttonsText->update_theme();
    simpleButton0->update_theme();
    simpleButton1->update_theme();
    imageButton->update_theme();
    imageRightTextButton->update_theme();
    imageBottomTextButton->update_theme();
    switcherButton->update_theme();
    radioButton0->update_theme();
    radioButton1->update_theme();
    anchorButton->update_theme();
}

void ButtonSheet::End()
{
    auto parentWindow = parentWindow_.lock();
    if (parentWindow)
    {
        parentWindow->remove_control(sheetButton1);
        parentWindow->remove_control(sheetButton0);
        parentWindow->remove_control(anchorButton);
        parentWindow->remove_control(radioButton1);
        parentWindow->remove_control(radioButton0);
        parentWindow->remove_control(switcherButton);
        parentWindow->remove_control(imageBottomTextButton);
        parentWindow->remove_control(imageRightTextButton);
        parentWindow->remove_control(imageButton);
        parentWindow->remove_control(simpleButton1);
        parentWindow->remove_control(simpleButton0);
        parentWindow->remove_control(buttonsText);
    }
}

void ButtonSheet::UpdateSize(int32_t width, int32_t height)
{
}