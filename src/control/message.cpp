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
    result_(message_result::undef),
	window(new wui::window()),
    //icon(new image()),
    text_(new text("", theme_)),
    button0(new button("", std::bind(&message::button0_click, this), theme_)),
    button1(new button("", std::bind(&message::button1_click, this), theme_)),
    button2(new button("", std::bind(&message::button2_click, this), theme_))
{
    window->set_transient_for(transient_window_, docked_);
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
    //window->add_control(icon, { 0 });
    text_->set_text(message_);

    icon_ = icon__;
    button_ = button__;
    result_callback_ = result_callback_;

    auto text_size = get_text_size();

    auto width = text_size.width() + 100;
    auto height = text_size.height() + 100;

    window->add_control(text_, { 80, 40, 80 + text_size.width(), 50 + text_size.height() });

    switch (button_)
    {
        case message_button::ok:
        {
            auto btn_width = 100;
            auto btn_height = 25;
            auto left = (width - btn_width) / 2;
            auto top = height - btn_height - 20;

            button0->set_caption(locale("button", "ok"));
            window->add_control(button0, { left, top, left + btn_width , top + btn_height });
        }
        break;
    }

    window->init(title_, { 0, 0, width, height }, window_style::dialog, [this]() { /*window.reset();*/ }, theme_);
}

message_result message::get_result() const
{
    return result_;
}

void message::button0_click()
{
    window->destroy();
}

void message::button1_click()
{
    window->destroy();
}

void message::button2_click()
{
    window->destroy();
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

    auto text_size = mem_gr.measure_text(max_line, theme_font(tc, tv_font, theme_));

    return { 0, 0, text_size.width(), static_cast<int32_t>(text_size.height() * 1.2 * lines_count) };
}

}
