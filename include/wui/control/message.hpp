//
// Copyright (c) 2021-2026 Intent Garden Org
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
//

#pragma once

#include <wui/window/window.hpp>

#include <wui/control/image.hpp>
#include <wui/control/text.hpp>
#include <wui/control/button.hpp>

namespace wui
{

enum class message_icon
{
    information,
    question,
    alert,
    stop
};

enum class message_button
{
    ok,
    ok_cancel,
    abort_retry_ignore,
    yes_no,
    yes_no_cancel,
    retry_cancel,
    cancel_try_continue
};

enum class message_result
{
    undef,
    ok,
    cancel,
    yes,
    no,
    abort,
    retry,
    ignore,
    try_,
    continue_
};

class message
{
public:
    message(std::shared_ptr<window> transient_window_,
        bool docked_ = true,
        std::shared_ptr<i_theme> theme_ = nullptr);
    ~message();

    void show(std::string_view message_,
        std::string_view title_,
        const message_icon icon_,
        const message_button button_,
        std::function<void(message_result)> result_callback = [](message_result) {});

    message_result get_result() const noexcept;
    void set_docked(const bool docked = true);

protected:
    std::string message_;
    message_icon icon_;
    message_button button_;
    std::function<void(message_result)> result_callback;
    std::shared_ptr<window> transient_window_;
    std::shared_ptr<i_theme> theme_;

    std::shared_ptr<window> window_;

    std::shared_ptr<image> icon;
    std::shared_ptr<text> text_;
    std::shared_ptr<button> button0, button1, button2;

    bool docked_;
    bool ctrl_pos_inited_{ false };

    rect icon_position_{ };
    rect button0_position_{ };
    rect button1_position_{ };
    rect button2_position_{ };
    rect text_position_{ };

    message_result result_;

    void calc_ctrl_position(int32_t &width_, int32_t &height_);
    void add_controls();
    void button0_click();
    void button1_click();
    void button2_click();
};

}
