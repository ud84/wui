//
// Copyright (c) 2023 Anton Golovkov (udattsk at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
//
//

#pragma once

#include <wui/window/window.hpp>

#include <wui/control/image.hpp>
#include <wui/control/text.hpp>
#include <wui/control/button.hpp>

#include <wui/control/scroll.hpp>

#include <wui/common/about.hpp>
#include <wui/system/uri_tools.hpp>
#include <wui/locale/locale.hpp>

#include <Resource.h>

class MainSheet
{
public:
    MainSheet();

    void Run(std::weak_ptr<wui::window> parentWindow_);
    void End();

    void UpdateSize(const int32_t width, const int32_t height, const bool top_only_update = false);

    static constexpr int32_t sheets_height = 30;

private:
    int32_t value_{ 8 };
    void IncSizeTextRect();
    void DecSizeTextRect();
    void redraw(const bool top_only_update = false);

    std::weak_ptr<wui::window> parentWindow_;

    std::shared_ptr<wui::image> logoImage = std::make_shared<wui::image>(IMG_LOGO);
    std::shared_ptr<wui::text_ex> welcomeText = std::make_shared<wui::text_ex>(
        wui::locale("main_sheet", "main_title"),
        wui::hori_alignment::left, wui::vert_alignment::center, "h1_text");

    std::shared_ptr<wui::button> switcherClip = std::make_shared<wui::button>("Clip",
        [this]() {
            wuiLeftTopText->set_clipping(switcherClip->turned());
            wuiTopTopText->set_clipping(switcherClip->turned());
            wuiTopCenterText->set_clipping(switcherClip->turned());
            wuiCenterText->set_clipping(switcherClip->turned());
            wuiCenterBottomText->set_clipping(switcherClip->turned());
            redraw();
        }, wui::button_view::switcher);

    std::shared_ptr<wui::text> wuiInfoText =
        std::make_shared<wui::text>(wui::about::full_name + std::string("\n")
        + wui::about::version
#if defined(_DEBUG)
         + " [Debug]"
#endif
#if defined(_UI_CHECK)
         + " [Debug UI]"
#endif
        , wui::hori_alignment::left, wui::vert_alignment::top);

    std::shared_ptr<wui::button> button_inc = std::make_shared<wui::button>("+", nullptr, wui::button_view::text);
    std::shared_ptr<wui::button> button_dec = std::make_shared<wui::button>("-", nullptr, wui::button_view::text);

    // sample: class text enable clipping (no alpha channel)
    std::shared_ptr<wui::text> wuiLeftTopText = std::make_shared<wui::text>("Left, top text.\nDemo clipping [on/off]",
        wui::hori_alignment::left, wui::vert_alignment::top,
        wui::text::tc, nullptr, true);

    std::shared_ptr<wui::text_ex> wuiTopTopText = std::make_shared<wui::text_ex>("Center, top text_ex", wui::hori_alignment::center, wui::vert_alignment::top);
    std::shared_ptr<wui::text_ex> wuiTopCenterText = std::make_shared<wui::text_ex>("Right, top text_ex", wui::hori_alignment::right, wui::vert_alignment::top);
    std::shared_ptr<wui::text_ex> wuiCenterText = std::make_shared<wui::text_ex>("Center, center text_ex", wui::hori_alignment::center, wui::vert_alignment::center);
    std::shared_ptr<wui::text_ex> wuiCenterBottomText = std::make_shared<wui::text_ex>("Center, bottom text_ex", wui::hori_alignment::center, wui::vert_alignment::bottom);

    std::shared_ptr<wui::button> mainSiteAnchor = std::make_shared<wui::button>(wui::about::web, []() { wui::open_uri(wui::about::web); }, wui::button_view::anchor);

    std::shared_ptr<wui::scroll> vertScroll = std::make_shared<wui::scroll>(1000, 0, wui::orientation::vertical,
        [this](wui::scroll_state, int32_t v) {  welcomeText->set_text<false>(std::to_string(horScroll->get_scroll_pos()) + std::string(", ") + std::to_string(v)); redraw(true); });
    std::shared_ptr<wui::scroll> horScroll = std::make_shared<wui::scroll>(1000, 1000, wui::orientation::horizontal,
        [this](wui::scroll_state, int32_t v) {  welcomeText->set_text<false>(std::to_string(v) + std::string(", ") + std::to_string(vertScroll->get_scroll_pos())); redraw(true); });
};
