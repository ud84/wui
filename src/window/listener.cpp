//
// Copyright (c) 2025 Anton Golovkov (udattsk at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/ud84/wui
//

#include <wui/window/listener.h>


namespace wui
{

listener::listener()
    : started(false),
    thread{},
    context_{},
    windows{},
    err{}
{
}

listener::~listener()
{
    stop();
}

void listener::add_window(xcb_window_t id, std::shared_ptr<window> window)
{
    auto w = windows.find(id);
    if (w == windows.end())
    {
        windows[id] = std::move(window);
    }

    if (windows.size() == 1) // Starting on first window
    {
        start();
    }
}

void listener::delete_window(xcb_window_t id)
{
    auto w = windows.find(id);
    if (w != windows.end())
    {
        windows.erase(w);
    }

    if (windows.empty())
    {
        started = false;
    }
}

bool listener::init()
{
    context_.display = XOpenDisplay(NULL);
    if (!context_.display)
    {
        err.type = error_type::system_error;
        err.component = "listener::start()";
        err.message = "Can't make the connection to X server";
        
        return false;
    }

    XSetEventQueueOwner(context_.display, XCBOwnsEventQueue);
    context_.connection = XGetXCBConnection(context_.display);

    context_.screen = xcb_setup_roots_iterator(xcb_get_setup(context_.connection)).data;

    return true;
}

void listener::start()
{
    if (started)
    {
        return;
    }

    started = true;
    thread = std::thread(std::bind(&listener::process_events, this));
}

void listener::stop()
{
    started = false;
    if (thread.joinable()) thread.join();

    XCloseDisplay(context_.display);
}

system_context const &listener::context() const
{
    return context_;
}

void listener::process_events()
{
    xcb_generic_event_t *e = nullptr;
    while (started && (e = xcb_wait_for_event(context_.connection)))
    {
        xcb_window_t w = e->pad[2];

        switch (e->response_type & ~0x80)
        {
            case XCB_EXPOSE:
            {
                auto ev = (xcb_expose_event_t*)e;
                w = ev->window;
            }
            break;
            case XCB_CONFIGURE_NOTIFY:
            case XCB_PROPERTY_NOTIFY:
            case XCB_CLIENT_MESSAGE:
            {
                auto ev = (xcb_configure_notify_event_t*)&e;
                w = e->pad[0];
            }
            break;
            default: break;
        }

        auto wnd = windows.find(w);
        if (wnd != windows.end())
        {
            wnd->second->process_events(*e);
        }

        free(e);
    }
}

error const &listener::get_error() const
{
    return err;
}

listener& get_listener()
{
    static listener instance;
    return instance;
}

}
