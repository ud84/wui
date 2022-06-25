//
// Copyright (c) 2021-2022 Anton Golovkov (udattsk at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/ud84/wui
//

#pragma once

#include <wui/event/event.hpp>
#include <wui/window/window.hpp>

#include <string>
#include <functional>
#include <memory>
#include <vector>

namespace wui
{

enum class tray_icon_action
{
    left_click,
    right_click,
    center_click,
    message_click
};

class tray_icon : public std::enable_shared_from_this<tray_icon>
{
public:
#ifdef _WIN32
    tray_icon(std::weak_ptr<window> parent, int32_t icon_resource_index, const std::string &tip, std::function<void(tray_icon_action action)> click_callback);
#endif
    tray_icon(std::weak_ptr<window> parent, const std::string &icon_file_name, const std::string &tip, std::function<void(tray_icon_action action)> click_callback);
    ~tray_icon();

#ifdef _WIN32
    void change_icon(int32_t icon_resource_index);
#endif
    void change_icon(const std::string &icon_file_name);

    void change_tip(const std::string &tip);

    void show_message(const std::string &title, const std::string &message);

    void set_callback(std::function<void(tray_icon_action action)> click_callback);

private:
    static const uint32_t ID = 3556;

    std::weak_ptr<window> parent;
#ifdef _WIN32
    int32_t icon_resource_index;
#endif
    std::string icon_file_name;
    std::string tip;
    std::function<void(tray_icon_action action)> click_callback;

    std::string my_subscriber_id;

    void receive_event(const event &ev);
};

}
