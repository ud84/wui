//
// Copyright (c) 2021-2026 Intent Garden Org
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
//

#ifdef _WIN32

#include <wui/framework/framework_win_impl.hpp>

#include <wui/window/listener.hpp>

#include <windows.h>

namespace wui
{

namespace framework
{
extern std::shared_ptr<listener> get_listener();

framework_win_impl::framework_win_impl()
    : started_(false)
{
}

void framework_win_impl::run()
{
    if (started_)
    {
        err.set(error_type::already_started, "framework_win_impl::run()");
        return;
    }
    started_ = true;

    MSG msg;
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    started_ = false;
}

void framework_win_impl::stop()
{
    if (started_)
    {
        PostQuitMessage(IDCANCEL);
        started_ = false;

        // err.reset(); // сохраняем ошибки
    }
}

bool framework_win_impl::started() const
{
    return started_;
}

error framework_win_impl::get_error() const
{
    return err;
}

}

}

#endif
