//
// Copyright (c) 2021-2022 Anton Golovkov (udattsk at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/ud84/wui
//

#include <wui/control/message.hpp>
#include <wui/locale/locale.hpp>
#include <wui/theme/theme.hpp>

#include <cstring>
#include <vector>
#include <sstream>

namespace wui
{

message::message(std::shared_ptr<wui::window> transient_window__,
    bool docked__,
    std::shared_ptr<i_theme> theme__)
    : icon_(message_icon::information),
    button_(message_button::ok),
    result_callback(),
    transient_window_(transient_window__), docked_(docked__),
    theme_(theme__),
	window_(new wui::window()),
    icon(new image(theme_image("message_info", theme_))),
    text_(new text("", theme_)),
    button0(new button("", std::bind(&message::button0_click, this), theme_)),
    button1(new button("", std::bind(&message::button1_click, this), theme_)),
    button2(new button("", std::bind(&message::button2_click, this), theme_)),
    result_(message_result::undef)
{
    window_->set_transient_for(transient_window_, docked_);
}

message::~message()
{
}

void message::show(const std::string &message_,
    const std::string &title_,
    message_icon icon__,
    message_button button__,
    std::function<void(message_result)> result_callback_)
{
    icon_ = icon__;
    button_ = button__;
    result_callback = result_callback_;

    result_ = message_result::undef;

    text_->set_text(message_);

    switch (icon_)
    {
        case message_icon::alert:
            icon->change_image(theme_image("message_alert", theme_));
        break;
        case message_icon::information:
            icon->change_image(theme_image("message_info", theme_));
        break;
        case message_icon::question:
            icon->change_image(theme_image("message_question", theme_));
        break;
        case message_icon::stop:
            icon->change_image(theme_image("message_stop", theme_));
        break;
    }

    auto text_size = get_text_size();

    auto width = text_size.width() + 110;
    auto height = text_size.height() + 120;

    window_->add_control(icon, { 20, 50, 68, 98 });

    window_->add_control(text_, { 90, 40, 90 + text_size.width(), 50 + text_size.height() });

    auto btn_width = 100;
    auto btn_height = 25;
    auto top = height - btn_height - 20;

    switch (button_)
    {
        case message_button::ok:
        {
            if (width <= btn_width)
            {
                width = btn_width * 2;
            }

            auto left = (width - btn_width) / 2;

            button0->set_caption(locale("button", "ok"));
            window_->add_control(button0, { left, top, left + btn_width , top + btn_height });
        }
        break;
        case message_button::ok_cancel: case message_button::yes_no: case message_button::retry_cancel:
        {
            if (width <= (btn_width + 10) * 2)
            {
                width = (btn_width + 10) * 3;
            }

            auto left = (width - (btn_width + 10) * 2) / 2;

            std::string btn0_caption = "ok", btn1_caption = "cancel";
            switch (button_)
            {
                case message_button::yes_no:
                    btn0_caption = "yes", btn1_caption = "no";
                break;
                case message_button::retry_cancel:
                    btn0_caption = "retry", btn1_caption = "cancel";
                break;
            }
            
            button0->set_caption(locale("button", btn0_caption));
            window_->add_control(button0, { left, top, left + btn_width , top + btn_height });

            button1->set_caption(locale("button", btn1_caption));
            window_->add_control(button1, { left + btn_width + 20, top, left + (btn_width * 2) + 20, top + btn_height });
        }
        break;
        case message_button::abort_retry_ignore: case message_button::cancel_try_continue:
        {
            if (width <= (btn_width + 10) * 3)
            {
                width = (btn_width + 10) * 4;
            }

            auto left = (width - (btn_width + 10) * 3) / 2;

            std::string btn0_caption = "abort", btn1_caption = "retry", btn2_caption = "ignore";
            if (button_ == message_button::cancel_try_continue)
            {
                btn0_caption = "cancel", btn1_caption = "try", btn2_caption = "continue";
            }

            button0->set_caption(locale("button", btn0_caption));
            window_->add_control(button0, { left, top, left + btn_width , top + btn_height });

            button1->set_caption(locale("button", btn1_caption));
            window_->add_control(button1, { left + btn_width + 20, top, left + (btn_width * 2) + 20, top + btn_height });

            button2->set_caption(locale("button", btn2_caption));
            window_->add_control(button2, { left + (btn_width * 2) + 40, top, left + (btn_width * 3) + 40, top + btn_height });
        }
        break;
    }

    window_->init(title_, { 0, 0, width, height }, window_style::dialog, [this]() {
        if (result_callback)
        {
            result_callback(result_);
        }
    }, theme_);
}

message_result message::get_result() const
{
    return result_;
}

void message::button0_click()
{
    switch (button_)
    {
        case message_button::ok: case message_button::ok_cancel:
            result_ = message_result::ok;
        break;
        case message_button::abort_retry_ignore:
            result_ = message_result::abort;
        break;
        case message_button::yes_no: case message_button::yes_no_cancel:
            result_ = message_result::yes;
        break;
        case message_button::retry_cancel:
            result_ = message_result::retry;
        break;
        case message_button::cancel_try_continue:
            result_ = message_result::cancel;
        break;
    }
    window_->destroy();
}

void message::button1_click()
{
    switch (button_)
    {
        case message_button::ok_cancel:
            result_ = message_result::cancel;
        break;
        case message_button::abort_retry_ignore:
            result_ = message_result::retry;
        break;
        case message_button::yes_no: case message_button::yes_no_cancel:
            result_ = message_result::no;
        break;
        case message_button::retry_cancel:
            result_ = message_result::cancel;
        break;
        case message_button::cancel_try_continue:
            result_ = message_result::try_;
        break;
    }
    window_->destroy();
}

void message::button2_click()
{
    switch (button_)
    {
        case message_button::abort_retry_ignore:
            result_ = message_result::ignore;
        break;
        case message_button::yes_no_cancel:
            result_ = message_result::cancel;
        break;
        case message_button::cancel_try_continue:
            result_ = message_result::continue_;
        break;
    }
    window_->destroy();
}

rect message::get_text_size()
{
    if (!transient_window_)
    {
        return { 0 };
    }

    std::stringstream text__(text_->get_text());
    std::string line, max_line;
    int32_t lines_count = 0;

    while (std::getline(text__, line, '\n'))
    {
        if (max_line.size() < line.size())
        {
            max_line = line;
        }
        ++lines_count;
    }

#ifdef _WIN32
    system_context ctx = { transient_window_->context().hwnd, GetDC(transient_window_->context().hwnd) };
#elif __linux__
    system_context ctx = transient_window_->context();
#endif

    graphic mem_gr(ctx);
    mem_gr.init(transient_window_->position(), 0);

    auto text_size = mem_gr.measure_text(max_line, theme_font(text::tc, text::tv_font, theme_));

    return { 0, 0, text_size.width(), static_cast<int32_t>(text_size.height() * 1.2 * lines_count) };
}

}