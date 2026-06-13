//
// Copyright (c) 2023 Anton Golovkov (udattsk at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
//
//

#include <ButtonSheet/ButtonSheet.h>

#include <Resource.h>

#include <wui/common/about.hpp>
#include <wui/locale/locale.hpp>
#include <wui/system/tools.hpp>
#include <wui/system/uri_tools.hpp>

#include <iostream>

ButtonSheet::ButtonSheet()
    : parentWindow_(),
    buttonsText(std::make_shared<wui::text>("Click on the buttons\n\t\tbelow the text", wui::hori_alignment::left, wui::vert_alignment::top, "h1_text")),
    simpleButton0(std::make_shared<wui::button>(wui::locale("button_sheet", "simple0"), [this]() { buttonsText->set_text("First button pushed"); }, wui::button_view::text)),
    simpleButton1(std::make_shared<wui::button>(wui::locale("button_sheet", "simple1"), [this]() { buttonsText->set_text("Second button pushed"); }, wui::button_view::text)),
    imageButton(std::make_shared<wui::button>("", [this]() { buttonsText->set_text("Image button pushed"); }, wui::button_view::image, IMG_LOGO, 32)),
    imageRightTextButton(std::make_shared<wui::button>(wui::locale("button_sheet", "right_text"), [this]() { buttonsText->set_text("Image with right text button pushed"); }, wui::button_view::image_right_text, IMG_LOGO, 32)),
    imageBottomTextButton(std::make_shared<wui::button>(wui::locale("button_sheet", "bottom_text"), [this]() { buttonsText->set_text("Image with bottom text button pushed"); }, wui::button_view::image_bottom_text, IMG_LOGO, 32, "rounded__green_button")),
    switcherButton(std::make_shared<wui::button>(wui::locale("button_sheet", "switcher_text"), [this]() { buttonsText->set_text("Switcher button pushed"); }, wui::button_view::switcher)),
    radioButton0(std::make_shared<wui::button>(wui::locale("button_sheet", "radio0_text"), [this]() { buttonsText->set_text("Radio first pushed"); radioButton1->turn(!radioButton0->turned()); }, wui::button_view::radio)),
    radioButton1(std::make_shared<wui::button>(wui::locale("button_sheet", "radio1_text"), [this]() { buttonsText->set_text("Radio second pushed"); radioButton0->turn(!radioButton1->turned()); }, wui::button_view::radio)),
    anchorButton(std::make_shared<wui::button>(wui::locale("button_sheet", "anchor_text"), [this]() { buttonsText->set_text("Anchor pushed"); }, wui::button_view::anchor)),
    sheetButton0(std::make_shared<wui::button>(wui::locale("button_sheet", "sheet0_text"), [this]() { buttonsText->set_text("First sheet pushed"); sheetButton0->turn(true); sheetButton1->turn(false); }, wui::button_view::sheet)),
    sheetButton1(std::make_shared<wui::button>(wui::locale("button_sheet", "sheet1_text"), [this]() { buttonsText->set_text("Second sheet pushed"); sheetButton0->turn(false); sheetButton1->turn(true); }, wui::button_view::sheet))
{
    radioButton0->turn(true);
    sheetButton0->turn(true);

    //buttonsText->set_clipping(false); // test

    auto e = std::move(imageButton->get_error());
    if (!e.is_ok())
    {
        std::cerr << e.str() << std::endl;
    }
}

void ButtonSheet::Run(std::weak_ptr<wui::window> parentWindow__)
{
    parentWindow_ = parentWindow__;

    auto parentWindow = parentWindow_.lock();
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

    if (parentWindow)
    {
        constexpr int32_t space = 25;
        const auto parentPos = parentWindow->position();
        const auto width = parentPos.width();

        int32_t top = 80;
        const int32_t text_height = buttonsText->get_preferred_size().height(); // -10 test clip on/off
        parentWindow->add_control(buttonsText, { 10, top, width - 10, top + text_height });
        top += text_height + space + 30;

        wui::rect r = simpleButton0->get_preferred_size();
        parentWindow->add_control(simpleButton0, { 10, top, 10 + r.width(), top + r.height() });
        wui::rect r1 = simpleButton1->get_preferred_size();
        parentWindow->add_control(simpleButton1, { 20 + r.width(), top, 20 + r.width() + r1.width(), top + r1.height() });
        top += r1.height() + space;

        r = imageButton->get_preferred_size();
        r1 = imageRightTextButton->get_preferred_size();
        int32_t h = wui::rect::max(r, r1).height();
        parentWindow->add_control(imageButton, { 10, top, 10 + r.width(), top + h });
        parentWindow->add_control(imageRightTextButton, { 20 + r.width(), top, 20 + r.width() + r1.width(), top + h });
        top += h + space;

        r = imageBottomTextButton->get_preferred_size();
        parentWindow->add_control(imageBottomTextButton, { 10, top, 10 + r.width(), top + r.height() });
        top += r.height() + space;

        r = switcherButton->get_preferred_size();
        parentWindow->add_control(switcherButton, { 10, top, 10 + r.width(), top + r.height() });
        top += r.height() + space;

        r = radioButton0->get_preferred_size();
        parentWindow->add_control(radioButton0, { 10, top, 10 + r.width(), top + r.height() });
        r1 = radioButton1->get_preferred_size();
        parentWindow->add_control(radioButton1, { 20 + r.width(), top, 20 + r.width() + r1.width(), top + r1.height() });
        top += r.height() + space;

        r = anchorButton->get_preferred_size();
        parentWindow->add_control(anchorButton, { 10, top, 10 + r.width(), top + r.height() });
        top += r.height() + space;

        r = sheetButton0->get_preferred_size();
        parentWindow->add_control(sheetButton0, { 10, top, 10 + r.width(), top + r.height() });
        r1 = sheetButton1->get_preferred_size();
        parentWindow->add_control(sheetButton1, { 20 + r.width(), top, 20 + r.width() + r1.width(), top + r1.height() });
    }

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

void ButtonSheet::UpdateSize(int32_t width [[maybe_unused]], int32_t height [[maybe_unused]] )
{
}