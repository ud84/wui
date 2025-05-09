//
// Copyright (c) 2023 Anton Golovkov (udattsk at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/ud84/wui
//

#include <wui/framework/framework.hpp>

#include <wui/framework/framework_win_impl.hpp>
#include <wui/framework/framework_lin_impl.hpp>

#include <wui/framework/i_framework.hpp>

#ifdef _WIN32
#include <windows.h>
#include <gdiplus.h>
#endif

#include <memory>
#include <iostream>

namespace wui
{

namespace framework
{

static std::shared_ptr<i_framework> instance = nullptr;

/// Interface

void init()
{
#ifdef _WIN32
        Gdiplus::GdiplusStartupInput gdiplusStartupInput;
        ULONG_PTR gdiplusToken;
        Gdiplus::GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);
#elif __linux__
        if (setlocale(LC_ALL, "") == NULL)
        {
            std::cerr << "warning: could not set default locale" << std::endl;
        }
#endif
}

void run()
{
    if (instance)
    {
        return;
    }
#ifdef _WIN32
    instance = std::make_shared<framework_win_impl>();
#elif __linux__
    instance = std::make_shared<framework_lin_impl>();
#elif __APPLE__
    instance = std::make_shared<framework_mac_impl>();
#endif

    instance->run();
}

void stop()
{
    if (instance)
    {
        instance->stop();
    }
    instance.reset();
}

bool started()
{
    return instance != nullptr;
}

error get_error()
{
    if (instance)
    {
        return instance->get_error();
    }
    return {};
}

}

}
