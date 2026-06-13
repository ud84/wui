//
// Copyright (c) 2021-2026 Intent Garden Org
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
//

#ifdef __linux__

#include <wui/framework/framework_lin_impl.hpp>

#include <wui/window/listener.hpp>

#include <thread>

namespace wui
{

namespace framework
{
extern std::shared_ptr<listener> get_listener();

framework_lin_impl::framework_lin_impl()
    : started_(false)
{
}

// call: main thread
void framework_lin_impl::run()
{
    if (started_)
    {
        err.set(error_type::already_started, "framework_lin_impl::run()");
        return;
    }

    started_ = true;

    while (started_)
    {
        std::unique_lock<std::mutex> lm{ mtx_cv_ };
        cv_.wait(lm);
    }

    auto listener__ = framework::get_listener();
    if (listener__)
        listener__->stop(); // не обязательно, но помогает отладке
}

// call: X11 listener::thread
void framework_lin_impl::stop()
{
    std::unique_lock<std::mutex> lm{ mtx_cv_ };
    started_ = false;
    cv_.notify_one();
}

bool framework_lin_impl::started() const
{
    return started_;
}

error framework_lin_impl::get_error() const
{
    return err;
}

}

}

#endif
