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

namespace wui
{

message::message(const std::string &message_,
    const std::string &title_,
    message_icon icon_,
    message_button button_,
    std::function<void(message_result)> result_callback,
    std::shared_ptr<wui::window> transient_window_, bool docked_,
    std::shared_ptr<i_theme> theme_)
    : window(new wui::window()),
    transient_window(transient_window_),
    //icon(new image()),
    text_(new text("", theme_)),
    button0(new button("", std::bind(&message::button0_click, this), theme_)),
    button1(new button("", std::bind(&message::button1_click, this), theme_)),
    button2(new button("", std::bind(&message::button2_click, this), theme_))
{
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

message_result show_message(const std::string &message_,
	const std::string &title_,
	message_icon icon_,
	message_button button_,
	std::shared_ptr<window> transient_window_, bool docked_,
	std::shared_ptr<i_theme> theme_)
{
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

	return out_result;
}

}
