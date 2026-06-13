//
// Copyright (c) 2021-2026 Intent Garden Org
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
//

#include <wui/control/message.hpp>
#include <wui/locale/locale.hpp>
#include <wui/theme/theme.hpp>

#ifdef min
#   undef min
#endif
#ifdef max
#   undef max
#endif

namespace wui
{
constexpr int32_t space = 20;
constexpr int32_t space_half = space / 2;
constexpr int32_t btn_width = 80;
constexpr int32_t btn_height = 25;

message::message(std::shared_ptr<wui::window> transient_window__,
    bool docked__,
    std::shared_ptr<i_theme> theme__)
    : icon_(message_icon::information),
    button_(message_button::ok),
    result_callback(),
    transient_window_(transient_window__),
    theme_(theme__),
    window_(std::make_shared<window>(window::tc, theme_)),
    icon(std::make_shared<image>(theme_image("message_info", theme_))),
    text_(std::make_shared<text>("", hori_alignment::left, vert_alignment::top, text::tc, theme_)),
    button0(std::make_shared<button>("", std::bind(&message::button0_click, this), button::tc, theme_)),
    button1(std::make_shared<button>("", std::bind(&message::button1_click, this), button::tc, theme_)),
    button2(std::make_shared<button>("", std::bind(&message::button2_click, this), button::tc, theme_)),
    docked_(docked__),
    result_(message_result::undef)
{
}

message::~message()
{
}

void message::set_docked(const bool docked)
{
    docked_ = docked;
    window_->set_transient_for(transient_window_, docked);
}

void message::show(std::string_view message__,
    std::string_view title_,
    const message_icon icon__,
    const message_button button__,
    std::function<void(message_result)> result_callback_)
{
    if (window_->context().physical())
    {
        return;
    }

    message_ = message__;
    icon_ = icon__;
    button_ = button__;
    result_callback = result_callback_;

    result_ = message_result::undef;
    ctrl_pos_inited_ = false;

    text_->set_text<false>(message_);

    if (!transient_window_ || !transient_window_->context().physical())
    {
        docked_ = false;
    }

    window_->set_transient_for(transient_window_, docked_);

    switch (icon_) {
        case message_icon::alert:
            icon->change_image_raw("message_alert", theme_);
            break;
        case message_icon::information:
            icon->change_image_raw("message_info", theme_);
            break;
        case message_icon::question:
            icon->change_image_raw("message_question", theme_);
            break;
        case message_icon::stop:
            icon->change_image_raw("message_stop", theme_);
            break;
    }

    int32_t width_ = 10, height_ = 10;
    if(graphic::text_measurer_inited())
        calc_ctrl_position(width_, height_);
    // else dialog window is root [graphic not initialized]

    window_->subscribe(
        [this](const wui::event& e) {
            if (e.type & wui::event_type::internal) {
                switch (e.internal_event_.type) {
                    case wui::internal_event_type::window_created:
                    {
                        const rect tw_pos = transient_window_->position();
                        if (!ctrl_pos_inited_)
                        {
                            // dialog window is root [graphic not initialized ]
                            rect pos = window_->position();
                            int32_t width_ = 10, height_ = 10;
                            calc_ctrl_position(width_, height_);
                            pos.resize(width_, height_);
                            window_->set_position(pos);
                        }
                        add_controls();
                    }
                    break;
                }
            }
        }, wui::event_type::internal);

    constexpr window_style dialog_top = window_style::title_showed | window_style::topmost
        | window_style::close_button | window_style::moving | window_style::border_all;

    window_->init(title_, { 0, 0, width_, height_ },
                  docked_ ? window_style::dialog : dialog_top, [this]() {
            if (result_callback)
            {
                result_callback(result_);
            }
        });
}

message_result message::get_result() const noexcept
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
    window_->close();
}

void message::button1_click()
{
    switch (button_)
    {
        case message_button::ok_cancel: case message_button::retry_cancel:
            result_ = message_result::cancel;
        break;
        case message_button::abort_retry_ignore:
            result_ = message_result::retry;
        break;
        case message_button::yes_no: case message_button::yes_no_cancel:
            result_ = message_result::no;
        break;
        case message_button::cancel_try_continue:
            result_ = message_result::try_;
        break;
        default: break;
    }
    window_->close();
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
        default: break;
    }
    window_->close();
}

void message::calc_ctrl_position(int32_t& width_, int32_t& height_)
{
    if (ctrl_pos_inited_)
        return;

    ctrl_pos_inited_ = true;

    const auto _top = window_->caption_height() + space;
    const auto _icon_width = icon->width();
    const auto _icon_height = icon->height();
    const rect text_rect = text_->get_preferred_size();
    const auto _text_height = text_rect.height();

    const auto _icon_top = _icon_height > _text_height ? _top : _top + (_text_height - _icon_height) / 2;
    icon_position_ = rect{ space, _icon_top, space + _icon_width, _icon_top + _icon_height };

    const auto _text_top = _text_height > _icon_height ? _top : _top + (_icon_height - _text_height) / 2;
    const auto height = std::max(_text_top + _text_height, _icon_top + _icon_height) + btn_height + space * 2;
    const auto top = height - btn_height - space;
    auto width = _icon_width + space * 3 + text_rect.width();

    switch (button_) {
        case message_button::ok:
        {
            if (width <= btn_width) {
                width = btn_width * 2;
            }

            const auto left = (width - btn_width) / 2;
            button0_position_ = rect{ left, top, left + btn_width , top + btn_height };
        }
        break;
        case message_button::ok_cancel: case message_button::yes_no: case message_button::retry_cancel:
        {
            if (width <= (btn_width + space_half) * 2) {
                width = (btn_width + space_half) * 3;
            }

            const auto left = (width - (btn_width + space_half) * 2) / 2;

            button0_position_ = rect{ left, top, left + btn_width , top + btn_height };
            button1_position_ = rect{ left + btn_width + space, top, left + (btn_width * 2) + space, top + btn_height };
        }
        break;
        case message_button::abort_retry_ignore: case message_button::cancel_try_continue: case message_button::yes_no_cancel:
        {
            if (width <= (btn_width + space_half) * 3) {
                width = (btn_width + space_half) * 4;
            }

            const auto left = (width - (btn_width + space_half) * 3) / 2;
            button0_position_ = rect{ left, top, left + btn_width , top + btn_height };
            button1_position_ = rect{ left + btn_width + space, top, left + (btn_width * 2) + space, top + btn_height };
            button2_position_ = rect{ left + ((btn_width + space) * 2), top, left + (btn_width * 3) + space * 2, top + btn_height };
        }
        break;
    }

    const auto _text_left = (width + space + _icon_width - text_rect.width()) / 2;
    text_position_ = rect{ _text_left, _text_top, _text_left + text_rect.width(), _text_top + _text_height};
    width_ = width;
    height_ = height;
}

void message::add_controls()
{
    window_->add_control(icon, icon_position_);

    switch (button_) {
        case message_button::ok:
        {
            button0->set_caption(locale("button", "ok"));
            window_->add_control(button0, button0_position_);
        }
        break;
        case message_button::ok_cancel: case message_button::yes_no: case message_button::retry_cancel:
        {
            std::string btn0_caption = "ok", btn1_caption = "cancel";
            switch (button_) {
                case message_button::yes_no:
                    btn0_caption = "yes", btn1_caption = "no";
                    break;
                case message_button::retry_cancel:
                    btn0_caption = "retry", btn1_caption = "cancel";
                    break;
            }

            button0->set_caption(locale("button", btn0_caption));
            window_->add_control(button0, button0_position_);

            button1->set_caption(locale("button", btn1_caption));
            window_->add_control(button1, button1_position_);
        }
        break;
        case message_button::abort_retry_ignore: case message_button::cancel_try_continue: case message_button::yes_no_cancel:
        {
            std::string btn0_caption = "abort", btn1_caption = "retry", btn2_caption = "ignore";
            if (button_ == message_button::cancel_try_continue) {
                btn0_caption = "cancel", btn1_caption = "try", btn2_caption = "continue";
            } else if (button_ == message_button::yes_no_cancel) {
                btn0_caption = "yes", btn1_caption = "no", btn2_caption = "cancel";
            }

            button0->set_caption(locale("button", btn0_caption));
            window_->add_control(button0, button0_position_);

            button1->set_caption(locale("button", btn1_caption));
            window_->add_control(button1, button1_position_);

            button2->set_caption(locale("button", btn2_caption));
            window_->add_control(button2, button2_position_);
        }
        break;
    }

    window_->add_control(text_, text_position_);
    window_->set_focused(button0);
}

}
