//
// Copyright (c) 2021-2026 Intent Garden Org
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
//

#include <wui/framework/framework.hpp>

#include <wui/framework/framework_win_impl.hpp>
#if __linux__
#include <wui/framework/framework_lin_impl.hpp>
#endif

#include <wui/framework/i_framework.hpp>

#include <wui/window/listener.hpp>

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
static std::shared_ptr<listener> listener_;
static std::shared_ptr<i_framework> instance;

/// Interface

[[nodiscard]] std::shared_ptr <listener> get_listener()
{
    return listener_;
}

bool init()
{
#ifdef _WIN32
    CoInitializeEx(nullptr, COINIT_MULTITHREADED);

    Gdiplus::GdiplusStartupInput gdiplusStartupInput;
    ULONG_PTR gdiplusToken;
    Gdiplus::GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);

    // DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE автоматически подстраивает размер окна
    // WM_DPICHANGED можно использовать для изменения размера шрифта и ctrl
    SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE);

#elif __linux__
    if (setlocale(LC_ALL, "") == NULL)
    {
        std::cerr << "warning: could not set default locale" << std::endl;
    }
#endif

    listener_ = std::make_shared<listener>();
    auto ok = listener_->init();
    if (!ok)
    {
        std::cerr << "framework::init : listener init(). " << listener_->get_error().str() << std::endl;
        // TODO
        // error err(error_type::system_error, "framework::init : listener init()");
        return false;
    }
    return true;
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

#ifdef _WIN32
    CoUninitialize();
#endif
}

void stop()
{
    if (instance)
    {
        auto listener__ = get_listener();
        if (!listener__ || !listener__->count()) // добавлено для совместимости
        {
            instance->stop();
            instance.reset();
        }
    }
}

bool started()
{
    return instance && instance->started();
}

error get_error()
{
    if (instance)
    {
        return std::move(instance->get_error());
    }
    return {};
}

}

}
