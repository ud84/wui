//
// Copyright (c) 2023 Anton Golovkov (udattsk at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
//
//

#include <MainSheet/MainSheet.h>

#include <iostream>

MainSheet::MainSheet()
{
    switcherClip->turn(true);

    auto e = std::move(logoImage->get_error());
    if (!e.is_ok())
    {
        std::cerr << e.str() << std::endl;
    }
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
        parentWindow->add_control(switcherClip, { 0 });
        parentWindow->add_control(wuiInfoText, { 0 });

        parentWindow->add_control(button_inc, { 0 });
        parentWindow->add_control(button_dec, { 0 });

        parentWindow->add_control(wuiLeftTopText, { 0 });
        parentWindow->add_control(wuiTopTopText, { 0 });
        parentWindow->add_control(wuiTopCenterText, { 0 });
        parentWindow->add_control(wuiCenterText, { 0 });
        parentWindow->add_control(wuiCenterBottomText, { 0 });

        parentWindow->add_control(mainSiteAnchor, { 0 });

        parentWindow->add_control(vertScroll, { 0 });
        parentWindow->add_control(horScroll,  { 0 });

        UpdateSize(width, height);
    }

    logoImage->update_theme();
    welcomeText->update_theme();
    switcherClip->update_theme();
    mainSiteAnchor->update_theme();
    wuiInfoText->update_theme();

    button_inc->update_theme();
    button_dec->update_theme();

    wuiLeftTopText->update_theme();
    wuiTopTopText->update_theme();
    wuiTopCenterText->update_theme();
    wuiCenterText->update_theme();
    wuiInfoText->update_theme();

    vertScroll->update_theme();
    horScroll->update_theme();

    button_inc->set_callback_down([this]() { IncSizeTextRect(); });
    button_dec->set_callback_down([this]() { DecSizeTextRect(); });
}

void MainSheet::End()
{
    auto parentWindow = parentWindow_.lock();
    if (parentWindow)
    {
        parentWindow->remove_control(logoImage);
        parentWindow->remove_control(welcomeText);
        parentWindow->remove_control(switcherClip);
        parentWindow->remove_control(wuiInfoText);

        parentWindow->remove_control(button_inc);
        parentWindow->remove_control(button_dec);

        parentWindow->remove_control(wuiLeftTopText);
        parentWindow->remove_control(wuiTopTopText);
        parentWindow->remove_control(wuiTopCenterText);
        parentWindow->remove_control(wuiCenterText);
        parentWindow->remove_control(wuiCenterBottomText);

        parentWindow->remove_control(vertScroll);
        parentWindow->remove_control(horScroll);

        parentWindow->remove_control(mainSiteAnchor);
    }
}

void MainSheet::IncSizeTextRect()
{
    if (value_ >= 100)
        return;
    ++value_;
    redraw();
}

void MainSheet::DecSizeTextRect()
{
    const int32_t min_dec = -wuiLeftTopText->get_font_size() + 1;
    if (value_ <= min_dec)
        return;;
    --value_;
    redraw();
}

void MainSheet::redraw(const bool top_only_update)
{
    auto parentWindow = parentWindow_.lock();
    if (parentWindow) {
        auto parentPos = parentWindow->position();
        const auto width = parentPos.width(), height = parentPos.height();
        UpdateSize(width, height, top_only_update);
        parentWindow->redraw({ 0, 0, width, height }, true);
    }
}

void MainSheet::UpdateSize(const int32_t width, const int32_t height,
    const bool top_only_update)
{
    constexpr int32_t space = 10;
    constexpr int32_t space_top = 40;//space * 10;
    constexpr int32_t image_size = 100; // set small image size
    constexpr int32_t button_space = 4;

    auto parentWindow = parentWindow_.lock();
    int32_t top = 32 + sheets_height;
    if (parentWindow) {
        top = parentWindow->caption_height() + sheets_height + 4;
    }

    const int32_t welcome_height = welcomeText->get_preferred_size().height();

    const int32_t scroll_button_size = wui::scroll::get_buttons_size();
    const int32_t right_scroll = width - space;
    const int32_t right_text = right_scroll - scroll_button_size - space;

    const int32_t shift_top_image = (space_top * 2 + image_size - welcome_height) / 2;
    int32_t top_image = top + space_top;
    int32_t top_welcome = top + space_top;
    if (shift_top_image < 0)
    {
        // logoImage top
        top_image = top - shift_top_image;
    }
    else
    {
        // welcomeText top
        top_welcome = top + shift_top_image;
    }

    logoImage->set_position({ space, top_image, space + image_size, top_image + image_size });

    constexpr int32_t left = space * 2 + image_size;
    welcomeText->set_position({ left, top_welcome, right_text, top_welcome + welcome_height });

    if (top_only_update)
    {
        return;
    }

    const int32_t text_line_height = wuiTopTopText->get_font_size() + value_;
    const int32_t text_line_height2 = wuiLeftTopText->get_preferred_size().height() + value_;

    const wui::rect switcher_rect = switcherClip->get_preferred_size();
    int32_t next_top = top_image + image_size + 4;
    switcherClip->set_position({ left, next_top, left + switcher_rect.width(), next_top + switcher_rect.height() });
    next_top += switcher_rect.height() + space;

    const wui::rect button_rect = button_inc->get_preferred_size();
    const int32_t button_size_w = button_rect.right + 12;
    const int32_t button_size_h = button_rect.bottom;
    button_inc->set_position({ left, next_top, left + button_size_w, next_top + button_size_h });
    button_dec->set_position({ left + button_size_w + button_space,
        next_top, left + button_size_w + button_space + button_size_w, next_top + button_size_h });

    const int32_t right_text_demo = left + 200;
    next_top += button_size_h + 2 + space;
    wuiLeftTopText->set_position({ left, next_top, right_text_demo, next_top + text_line_height2 });
    next_top += text_line_height2 + space;
    wuiTopTopText->set_position({ left, next_top, right_text_demo, next_top + text_line_height });
    next_top += text_line_height + space;
    wuiTopCenterText->set_position({ left, next_top, right_text_demo, next_top + text_line_height });
    next_top += text_line_height + space;
    wuiCenterText->set_position({ left, next_top, right_text_demo, next_top + text_line_height });
    next_top += text_line_height + space;
    wuiCenterBottomText->set_position({ left, next_top, right_text_demo, next_top + text_line_height });

    const wui::rect butAnchor = mainSiteAnchor->get_preferred_size();
    const int32_t bottom_anchor_height = butAnchor.height();
    const int32_t bottom_anchor = height - (space + bottom_anchor_height);
    const int32_t bottom_scroll = bottom_anchor - space - scroll_button_size;

    const wui::rect info_rect = wuiInfoText->get_preferred_size();
    const int32_t bottom_info_height = info_rect.height(); // space + butAnchor
    const int32_t bottom_info = bottom_scroll - space - bottom_info_height;

    wuiInfoText->set_position({ space, bottom_info, space + info_rect.width(), bottom_info + bottom_info_height });

    horScroll->set_position({ space, bottom_scroll, right_scroll, bottom_scroll + scroll_button_size });

    vertScroll->set_position({ width - 50, top + space, right_scroll, bottom_scroll - 1 });

    mainSiteAnchor->set_position({ space, bottom_anchor, space + butAnchor.width(), bottom_anchor + bottom_anchor_height });
}
