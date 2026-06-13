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

#ifdef __linux__
#include <xcb/xcb.h>
#endif

namespace wui
{

#ifdef __linux__

// TODO:  i_listener, скрыть stop()
class listener
{
public:
    struct wnd
    {
        std::shared_ptr<window> window_;
        bool created{ false };

        [[nodiscard]] bool inited() const noexcept { return created && window_; }
    };

    listener() = default;
    ~listener();

    void add_window(xcb_window_t id, std::shared_ptr<window> window);
    void delete_window(xcb_window_t id);

    [[nodiscard]] bool init();
    [[nodiscard]] bool is_init() const;

    [[nodiscard]] bool empty() const noexcept
    {
        return windows.empty();
    }

    [[nodiscard]] size_t count() const noexcept
    {
        return windows.size();
    }

    [[nodiscard]] std::shared_ptr<window> get_first()
    {
        auto it = windows.begin();
        return it != windows.end() ? it->second.window_ : nullptr;
    }

    [[nodiscard]] std::shared_ptr<window> get_any_window(std::function<bool(const wnd&)> cmp);

    [[nodiscard]] system_context const &context() const;

    [[nodiscard]] error get_error() const;

    void stop();

private:

    std::atomic<bool> started{ false };
    std::thread thread;
    system_context context_;

    std::unordered_map<xcb_window_t, wnd> windows;

    error err;

    void start();

    void process_events();
};

#elif _WIN32

// Windows implementation
class listener
{
public:
    struct wnd
    {
        std::shared_ptr<window> window_;

        [[nodiscard]] bool inited() const noexcept { return nullptr != window_; }
    };

    listener() = default;
    ~listener() { }

    void add_window(void* hwnd, std::shared_ptr<window> window_)
    {
        auto w = windows.find(hwnd);
        if (w == windows.end())
        {
            windows[hwnd] = { std::move(window_) }; //, 0 == windows.size()
        };
    }

    void delete_window(void* hwnd)
    {
        auto w = windows.find(hwnd);
        if (w != windows.end())
        {
            windows.erase(w);
        }
    }

    constexpr bool init() const noexcept
    {
        return true;
    }

    [[nodiscard]] bool empty() const noexcept
    {
        return windows.empty();
    }

    [[nodiscard]] size_t count() const noexcept
    {
        return windows.size();
    }

    [[nodiscard]] std::shared_ptr<window> get_first()
    {
        auto it = windows.begin();
        return it != windows.end() ? it->second.window_ : nullptr;
    }

    [[nodiscard]] std::shared_ptr<window> get_any_window(std::function<bool(const wnd&)> cmp)
    {
        auto it{ std::find_if(windows.begin(), windows.end(), [cmp](const auto& n) -> bool { return cmp(n.second); }) };
        return it != windows.end() ? it->second.window_ : nullptr;
    }

    [[nodiscard]] system_context const &context() const { static system_context ctx{}; return ctx; }

    [[nodiscard]] error get_error() const { return err; }

private:
    std::unordered_map<void*, wnd> windows;

    error err;

    //void stop() { };
    //void start() { };
};


#endif

}
