//
// Copyright (c) 2025 Anton Golovkov (udattsk at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/ud84/wui
//

#include <wui/system/udev_handler.hpp>

#include <string>

#include <libudev.h>

#include <string.h>

namespace wui
{

udev_handler::udev_handler(std::function<void(const event &ev)> callback_)
    : callback(callback_),
    started(false),
    thread()
{
}

udev_handler::~udev_handler()
{
    stop();
}

void udev_handler::start()
{
    if (started) return;

    started = true;

    thread = std::thread(std::bind(&udev_handler::process, this));
}

void udev_handler::stop()
{
    if (!started) return;

    started = false;

    udev_monitor_unref(static_cast<struct udev_monitor*>(mon));

    if (thread.joinable()) thread.join();
}

void send_event(device_type dt, system_event_type et, std::function<void(const event &ev)> callback)
{
    system_event sev = { et,
        dt,
        0,
        0
    };
    event ev_;
    ev_.type = event_type::system;
    ev_.system_event_ = sev;
    callback(ev_);
}

void handle_device(struct udev_device *dev, std::function<void(const event &ev)> callback)
{
    const char *action  = udev_device_get_action(dev);
    const char *subsys  = udev_device_get_subsystem(dev);
    const char *devtype = udev_device_get_devtype(dev);

    if (!action  || (strcmp(action, "add") != 0 && strcmp(action, "remove") != 0)) return; // Only add or remove
    if (!subsys  || strcmp(subsys,  "usb") != 0)                                   return; // Only usb
    if (!devtype || strcmp(devtype, "usb_device") != 0)                            return; // Only root

    if (strcmp(action, "remove") == 0)
    {
        return send_event(device_type::usb, system_event_type::device_disconnected, callback); // It's workaround, need to be improve
    }

    const char *ifs = udev_device_get_property_value(dev,"ID_USB_INTERFACES");
    if (!ifs) return;

    bool has_cam = false, has_audio = false, has_hid = false;

    for (const char *p = strtok(strdup(ifs), ":"); p; p = strtok(NULL, ":"))
    {
        unsigned cls = 0;
        sscanf(p, "%2x", &cls);
        if (cls == 0x0e) has_cam = true;
        if (cls == 0x01) has_audio = true;
        if (cls == 0x03) has_hid = true;
    }

    if (has_cam) send_event(device_type::camera, system_event_type::device_connected, callback);
    if (has_audio) send_event(device_type::media, system_event_type::device_connected, callback);
    if (has_hid) send_event(device_type::hid, system_event_type::device_connected, callback);
}

void udev_handler::process()
{
    struct udev *udev = udev_new();
    mon = udev_monitor_new_from_netlink(udev, "udev");

    udev_monitor_filter_add_match_subsystem_devtype(static_cast<struct udev_monitor*>(mon), "usb", NULL);
    udev_monitor_enable_receiving(static_cast<struct udev_monitor*>(mon));

    int fd = udev_monitor_get_fd(static_cast<struct udev_monitor*>(mon));

    while (started)
    {
        fd_set fds;
        FD_ZERO(&fds);
        FD_SET(fd, &fds);

        int r = select(fd+1, &fds, NULL, NULL, NULL);

        if (r < 0 && errno == EBADF) break;

        if (FD_ISSET(fd, &fds))
        {
            struct udev_device *dev = udev_monitor_receive_device(static_cast<struct udev_monitor*>(mon));
            if (!dev)
            {
                continue;
            }

            auto action = udev_device_get_action(dev);

            if (!action)
            {
                continue;
            }

            if (dev)
            {
                handle_device(dev, callback);
                udev_device_unref(dev);
            }
        }
    }

    udev_unref(udev);
}

}
