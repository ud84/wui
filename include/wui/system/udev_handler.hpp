//
// Copyright (c) 2025 Anton Golovkov (udattsk at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/ud84/wui
//

#pragma once

#include <wui/event/event.hpp>
#include <wui/event/system_event.hpp>

#include <functional>
#include <thread>

namespace wui
{

class udev_handler
{
public:
    udev_handler(std::function<void(const event &ev)> callback);
    ~udev_handler();

    void start();
    void stop();

private:
    std::function<void(const event &ev)> callback;
    bool started;
    std::thread thread;

    void *mon;

    void process();
};

}
