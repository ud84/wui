//
// Copyright (c) 2025 Anton Golovkov (udattsk at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/ud84/wui
//

#pragma once

#include <wui/window/window.hpp>

#include <wui/system/system_context.hpp>
#include <wui/common/error.hpp>

#include <thread>
#include <unordered_map>

namespace wui
{

class listener
{
public:
    listener();
    ~listener();

    void add_window(xcb_window_t id, std::shared_ptr<window> window);
    void delete_window(xcb_window_t id);

    bool init();
    
    system_context const &context() const;

    error const &get_error() const;

private:
    bool started;
    std::thread thread;
    system_context context_;

    std::unordered_map<xcb_window_t, std::shared_ptr<window>> windows;

    error err;

    void start();
    void stop();
    
    void process_events();
};

listener& get_listener(); /// Singleton

}
