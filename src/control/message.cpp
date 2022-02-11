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

message::message(const std::string &message_,
    const std::string &title_,
    message_icon icon_,
    message_button button_,
    std::function<void(message_result)> result_callback,
    std::shared_ptr<wui::window> transient_window__, bool docked_,
    std::shared_ptr<i_theme> theme__)
    : theme_(theme__),
	window(new wui::window()), transient_window_(transient_window__),
    //icon(new image()),
    text_(new text(message_, theme_)),
    button0(new button("", std::bind(&message::button0_click, this), theme_)),
    button1(new button("", std::bind(&message::button1_click, this), theme_)),
    button2(new button("", std::bind(&message::button2_click, this), theme_))
{
    auto text_size = get_text_size();

    auto width = text_size.width() + 50;
    auto height = text_size.height() + 80;

	//window->add_control(icon, { 0 });
	window->add_control(text_, { 10, 30, 10 + text_size.width(), 30 + text_size.height() });
	window->add_control(button0, { 0 });

	window->set_transient_for(transient_window_, docked_);
	window->init(title_, { 0, 0, width, height }, window_style::dialog, [this]() { /*window.reset();*/ }, theme_);
}

message::~message()
{
}

void message::button0_click()
{

}

void message::button1_click()
{

}

void message::button2_click()
{

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

message_result show_message(const std::string &message_,
    const std::string &title_,
    message_icon icon_,
    message_button button_,
    std::shared_ptr<window> transient_window_, bool docked_,
    std::shared_ptr<i_theme> theme_)
{
    //message_result out_result = message_result::undef;

    std::thread([&message_, &title_, &icon_, &button_, &transient_window_, &docked_, &theme_]() {
        message_result out_result = message_result::undef;

        auto end_callback = [&out_result](message_result result) noexcept -> void
        {
            out_result = result;
        };

        message dialog(message_, title_, icon_, button_, end_callback, transient_window_, docked_, theme_);

        while (out_result == message_result::undef)
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(200));
        }
    }).detach();
    
    /*if (thread.joinable())
    {
        thread.join();
    }*/

    return message_result::undef;// out_result;
}

}
