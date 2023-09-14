//
// Copyright (c) 2021-2022 Anton Golovkov (udattsk at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/ud84/wui
//

#include <wui/window/window.hpp>

#include <wui/graphic/graphic.hpp>

#include <wui/theme/theme.hpp>
#include <wui/locale/locale.hpp>

#include <wui/control/button.hpp>

#include <wui/common/flag_helpers.hpp>

#include <wui/system/tools.hpp>
#include <boost/nowide/convert.hpp>

#include <algorithm>
#include <set>
#include <random>

#ifdef _WIN32

#include <windowsx.h>

#elif __linux__

#include <cstring>

#include <stdlib.h>
#include <xcb/xcb_ewmh.h>
#include <xcb/xcb_icccm.h>

#include <X11/Xutil.h>

#endif

// Some helpers
#ifdef _WIN32

void CenterHorizontally(wui::rect &pos, wui::system_context &context)
{
    RECT work_area;
    SystemParametersInfo(SPI_GETWORKAREA, 0, &work_area, 0);
    auto screen_width = work_area.right - work_area.left;
    pos.left = (screen_width - pos.right) / 2;
    pos.right += pos.left;
}

void CenterVertically(wui::rect &pos, wui::system_context &context)
{
    RECT work_area;
    SystemParametersInfo(SPI_GETWORKAREA, 0, &work_area, 0);
    auto screen_height = work_area.bottom - work_area.top;
    pos.top = (screen_height - pos.bottom) / 2;
    pos.bottom += pos.top;
}

#elif __linux__

void CenterHorizontally(wui::rect &pos, wui::system_context &context_)
{
    pos.left = (context_.screen->width_in_pixels - pos.right) / 2;
    pos.right += pos.left;
}

void CenterVertically(wui::rect &pos, wui::system_context &context_)
{
    pos.top = (context_.screen->height_in_pixels - pos.bottom) / 2;
    pos.bottom += pos.top;
}

void remove_window_decorations(wui::system_context &context)
{
    std::string mwh = "_MOTIF_WM_HINTS";
    xcb_intern_atom_reply_t *reply = xcb_intern_atom_reply(context.connection,
        xcb_intern_atom(context.connection, 0, mwh.size(), mwh.c_str()),
        NULL);
    auto mwh_atom = reply->atom;
    free(reply);

    struct motif_hints
    {
        uint32_t flags;
        uint32_t functions;
        uint32_t decorations;
        int32_t  input_mode;
        uint32_t status;
    };

    motif_hints hints = { 0 };
    hints.flags = 2;

    xcb_change_property(context.connection,
        XCB_PROP_MODE_REPLACE,
        context.wnd,
        mwh_atom,
        XCB_ATOM_WM_HINTS,
        32,
        5,
        &hints);
}

wui::rect get_window_size(wui::system_context &context)
{
    auto geom = xcb_get_geometry_reply(context.connection, xcb_get_geometry(context.connection, context.wnd), NULL);
    if (geom)
    {
        wui::rect out{ geom->x, geom->y, geom->x + geom->width, geom->y + geom->height };
        free(geom);

        return out;
    }
    return { 0 };
}

#endif

namespace wui
{

window::window(const std::string &theme_control_name, std::shared_ptr<i_theme> theme_)
    : context_{ 0 },
    graphic_(context_),
    controls(),
    active_control(),
    caption(),
    position_(), normal_position(),
    min_width(0), min_height(0),
    window_style_(window_style::frame),
    window_state_(window_state::normal), prev_window_state_(window_state_),
    tcn(theme_control_name),
    theme_(theme_),
    showed_(true), enabled_(true),
    focused_index(0),
    parent_(),
    my_control_sid(), my_plain_sid(),
    transient_window(), docked_(false), docked_control(),
    subscribers_(),
    moving_mode_(moving_mode::none),
    x_click(0), y_click(0),
    close_callback(),
    control_callback(),
    default_push_control(),
    switch_theme_button(new button(locale(tcn, cl_light_theme), std::bind(&window::switch_theme, this), button_view::image, theme_image(ti_switch_theme), 24, button::tc_tool)),
    pin_button(new button(locale(tcn, cl_pin), std::bind(&window::pin, this), button_view::image, theme_image(ti_pin), 24, button::tc_tool)),
    minimize_button(new button("", std::bind(&window::minimize, this), button_view::image, theme_image(ti_minimize), 24, button::tc_tool)),
    expand_button(new button("", [this]() { window_state_ == window_state::normal ? expand() : normal(); }, button_view::image, window_state_ == window_state::normal ? theme_image(ti_expand) : theme_image(ti_normal), 24, button::tc_tool)),
    close_button(new button("", std::bind(&window::destroy, this), button_view::image, theme_image(ti_close), 24, button::tc_tool_red)),
#ifdef _WIN32
    mouse_tracked(false)
#elif __linux__
    wm_protocols_event(), wm_delete_msg(), wm_change_state(), net_wm_state(), net_wm_state_focused(), net_wm_state_above(), net_wm_state_skip_taskbar(), net_active_window(),
    prev_button_click(0),
    runned(false),
    thread()
#endif
{
    switch_theme_button->disable_focusing();
    pin_button->disable_focusing();
    minimize_button->disable_focusing();
    expand_button->disable_focusing();
    close_button->disable_focusing();
}

window::~window()
{
    auto parent__ = parent_.lock();
    if (parent__)
    {
        parent__->remove_control(shared_from_this());
    }
#ifdef _WIN32
    if (context_.hwnd)
    {
        DestroyWindow(context_.hwnd);
    }
#elif __linux__
    send_destroy_event();
    if (thread.joinable()) thread.join();
#endif
}

void window::add_control(std::shared_ptr<i_control> control, const rect &control_position)
{
    if (std::find(controls.begin(), controls.end(), control) == controls.end())
    {
        control->set_parent(shared_from_this());
        control->set_position(control_position, false);
        controls.emplace_back(control);

        redraw(control->position());
    }
}

void window::remove_control(std::shared_ptr<i_control> control)
{
    if (!control)
    {
        return;
    }

    auto exists = std::find(controls.begin(), controls.end(), control);
    if (exists != controls.end())
    {
        controls.erase(exists);
    }

    if (control == docked_control)
    {
        docked_control.reset();
    }
    
    auto clear_pos = control->position();
    control->clear_parent();
    redraw(clear_pos, true);
}

void window::bring_to_front(std::shared_ptr<i_control> control)
{
    auto size = controls.size();
    if (size > 1)
    {
        auto it = std::find(controls.begin(), controls.end(), control);
        if (it != controls.end())
        {
            controls.erase(it);
        }
        controls.emplace_back(control);
    }
}

void window::move_to_back(std::shared_ptr<i_control> control)
{
    auto size = controls.size();
    if (size > 1)
    {
        auto it = std::find(controls.begin(), controls.end(), control);
        if (it != controls.end())
        {
            controls.erase(it);
        }
        controls.insert(controls.begin(), control);
    }
}

void window::redraw(const rect &redraw_position, bool clear)
{
    auto parent__ = parent_.lock();
    if (parent__)
    {
        parent__->redraw(redraw_position);
    }
    else
    {
#ifdef _WIN32
        RECT invalidatingRect = { redraw_position.left, redraw_position.top, redraw_position.right, redraw_position.bottom };
        InvalidateRect(context_.hwnd, &invalidatingRect, clear ? TRUE : FALSE);
#elif __linux__
        if (context_.connection)
        {
            xcb_expose_event_t event = { 0 };

            event.window = context_.wnd;
            event.response_type = XCB_EXPOSE;
            event.x = redraw_position.left;
            event.y = redraw_position.top;
            event.width = redraw_position.width();
            event.height = redraw_position.height();
            event.pad0 = clear ? 1 : 0;

            xcb_send_event(context_.connection, false, context_.wnd, XCB_EVENT_MASK_EXPOSURE, (const char*)&event);
            xcb_flush(context_.connection);
        }
#endif
    }
}

std::string window::subscribe(std::function<void(const event&)> receive_callback_, event_type event_types_, std::shared_ptr<i_control> control_)
{
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> uid(0, 61);

    auto randchar = [&uid, &gen]() -> char
    {
        const char charset[] =
            "0123456789"
            "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
            "abcdefghijklmnopqrstuvwxyz";
        const size_t max_index = (sizeof(charset) - 1);
        return charset[uid(gen)];
    };

    std::string id(20, 0);
    std::generate_n(id.begin(), 20, randchar);

    subscribers_.emplace_back(event_subscriber{ id, receive_callback_, event_types_, control_ });
    return id;
}

void window::unsubscribe(const std::string &subscriber_id)
{
    auto it = std::find_if(subscribers_.begin(), subscribers_.end(), [&subscriber_id](const event_subscriber &es) {
        return es.id == subscriber_id;
    });
    if (it != subscribers_.end())
    {
        subscribers_.erase(it);
    }
}

system_context &window::context()
{
    auto parent__ = parent_.lock();
    if (!parent__)
    {
        return context_;
    }
    else
    {
        return parent__->context();
    }
}

void window::draw(graphic &gr, const rect &paint_rect)
{
    /// drawing the child window

    if (!showed_)
    {
        return;
    }

    auto window_pos = position();

    gr.draw_rect(window_pos, theme_color(tcn, tv_background, theme_));

    if (flag_is_set(window_style_, window_style::title_showed))
    {
        gr.draw_text({ window_pos.left + 5, window_pos.top + 5, 0, 0 },
            caption,
            theme_color(tcn, tv_text, theme_),
            theme_font(tcn, tv_caption_font, theme_));
    }

    draw_border(gr);

    std::vector<std::shared_ptr<i_control>> topmost_controls;
    
    for (auto &control : controls)
    {
        if (control->position().in(paint_rect))
        {
            if (!control->topmost())
            {
                control->draw(gr, paint_rect);
            }
            else
            {
                topmost_controls.emplace_back(control);
            }
        }
    }

    for (auto &control : topmost_controls)
    {
        control->draw(gr, paint_rect);
    }
}

void window::receive_control_events(const event &ev)
{
    /// Here we receive events from the parent window and relay them to our child controls

    if (!showed_)
    {
        return;
    }

    switch (ev.type)
    {
        case event_type::mouse:
            send_mouse_event(ev.mouse_event_);
        break;
        case event_type::keyboard:
        {
            auto control = get_focused();
            if (control)
            {
                send_event_to_control(control, ev);
            }
        }
        break;
        case event_type::internal:
            switch (ev.internal_event_.type)
            {
                case internal_event_type::set_focus:
                    change_focus();
                break;
                case internal_event_type::remove_focus:
                {
                    size_t focusing_controls = 0;
                    for (const auto &control : controls)
                    {
                        if (control->focused())
                        {
                            event ev;
                            ev.type = event_type::internal;
                            ev.internal_event_ = internal_event{ internal_event_type::remove_focus };
                            send_event_to_control(control, ev);

                            ++focused_index;
                        }

                        if (control->focusing())
                        {
                            ++focusing_controls;
                        }
                    }

                    if (focused_index > focusing_controls)
                    {
                        focused_index = 0;
                    }
                }
                break;
                case internal_event_type::execute_focused:
                    execute_focused();
                break;
            }
        break;
        default: break;
    }
}

void window::receive_plain_events(const event &ev)
{
    if (ev.type == event_type::internal && ev.internal_event_.type == wui::internal_event_type::size_changed)
    {
        int32_t w = ev.internal_event_.x, h = ev.internal_event_.y;
        if (docked_)
        {
            int32_t left = (w - position_.width()) / 2;
            int32_t top = (h - position_.height()) / 2;

            auto new_position = position_;
            new_position.put(left, top);
            set_position(new_position, false);
        }

        return;
    }

    send_event_to_plains(ev);
}

void window::set_position(const rect &position__, bool redraw_)
{
    auto old_position = position_;
    auto position___ = position__;

#ifdef _WIN32
    if (context_.hwnd)
#elif __linux__
    if (context_.connection && context_.wnd)
#endif
    {
        if (position___.left == -1)
        {
            CenterHorizontally(position___, context_);
        }
        if (position___.top == -1)
        {
            CenterVertically(position___, context_);
        }

        position_ = position___;

#ifdef _WIN32
    SetWindowPos(context_.hwnd, NULL, position___.left, position___.top, position___.width(), position___.height(), NULL);
#elif __linux__
        uint32_t values[] = { static_cast<uint32_t>(position___.left), static_cast<uint32_t>(position___.top),
            static_cast<uint32_t>(position___.width()), static_cast<uint32_t>(position___.height()) };
        xcb_configure_window(context_.connection, context_.wnd, XCB_CONFIG_WINDOW_X | XCB_CONFIG_WINDOW_Y |
            XCB_CONFIG_WINDOW_WIDTH | XCB_CONFIG_WINDOW_HEIGHT, values);
#endif
        redraw({ 0, 0, position___.width(), position___.height() }, true);
    }

    if (parent_.lock())
    {
        auto parent_pos = parent_.lock()->position();

        auto left = position___.left;
        if (left == -1)
        {
            left = (parent_pos.width() - position___.width()) / 2;
        }
        auto top = position___.top;
        if (top == -1)
        {
            top = (parent_pos.height() - position___.height()) / 2;
        }

        position___ = { left, top, left + position___.width(), top + position___.height() };

        update_control_position(position_, position___, showed_ && redraw_, parent_);
    
        send_internal(internal_event_type::size_changed, position_.width(), position_.height());

        if (old_position.width() != position_.width())
        {
            update_buttons();
        }
    }
}

rect window::position() const
{
    return get_control_position(position_, parent_);
}

void window::set_parent(std::shared_ptr<window> window)
{
    parent_ = window;

    if (window)
    {
#ifdef _WIN32
        if (context_.hwnd)
        {
            DestroyWindow(context_.hwnd);
        }
#elif __linux__
        if (context_.display)
        {
            send_destroy_event();
        }
#endif

        my_control_sid = window->subscribe(std::bind(&window::receive_control_events, this, std::placeholders::_1), event_type::all, shared_from_this());
        my_plain_sid = window->subscribe(std::bind(&window::receive_plain_events, this, std::placeholders::_1), event_type::all);

        pin_button->set_caption(locale(tcn, cl_unpin));
    }
}

std::weak_ptr<window> window::parent() const
{
    return parent_;
}

void window::clear_parent()
{
    auto parent__ = parent_.lock();
    if (parent__)
    {
        parent__->unsubscribe(my_control_sid);
        parent__->unsubscribe(my_plain_sid);
    }

    parent_.reset();
}

void window::set_topmost(bool yes)
{
    if (yes)
    {
        set_style(static_cast<window_style>(static_cast<uint32_t>(window_style_) | static_cast<uint32_t>(window_style::topmost)));
    }
    else
    {
        set_style(static_cast<window_style>(static_cast<uint32_t>(window_style_) & ~static_cast<uint32_t>(window_style::topmost)));
    }
}

bool window::topmost() const
{
    return docked_ || parent_.lock() || flag_is_set(window_style_, window_style::topmost);
}

bool window::focused() const
{
    for (const auto &control : controls)
    {
        if (control->focused())
        {
            return true;
        }
    }

    return false;
}

bool window::focusing() const
{
    for (const auto &control : controls)
    {
        if (control->focusing())
        {
            return true;
        }
    }

    return false;
}

void window::update_theme_control_name(const std::string &theme_control_name)
{
    tcn = theme_control_name;
    update_theme(theme_);
}

void window::update_theme(std::shared_ptr<i_theme> theme__)
{
    if (theme_ && !theme__)
    {
        return;
    }
    theme_ = theme__;

    if (context_.valid() && !parent_.lock())
    {
        graphic_.set_background_color(theme_color(tcn, tv_background, theme_));

#ifdef _WIN32

        RECT client_rect;
        GetClientRect(context_.hwnd, &client_rect);
        InvalidateRect(context_.hwnd, &client_rect, TRUE);

#elif __linux__

        auto ws = get_window_size(context_);
        redraw({ 0, 0, ws.width(), ws.height() }, true);

#endif
    }

    for (auto &control : controls)
    {
        control->update_theme(theme_);
    }

    update_button_images();
}

void window::show()
{
    showed_ = true;

    if (!parent_.lock())
    {
#ifdef _WIN32
        ShowWindow(context_.hwnd, SW_SHOW);
#elif __linux__
        update_window_style();
#endif
    }
    else
    {
        for (auto &control : controls)
        {
            control->show();
        }
    }
}

void window::hide()
{
    showed_ = false;

    auto parent__ = parent_.lock();
    if (!parent__)
    {
#ifdef _WIN32
        ShowWindow(context_.hwnd, SW_HIDE);
#elif __linux__
        update_window_style();
#endif
    }
    else
    {
        for (auto &control : controls)
        {
            control->hide();
        }

        parent__->redraw(position(), true);
    }
}

bool window::showed() const
{
    return showed_;
}

void window::enable()
{
    enabled_ = true;

#ifdef _WIN32
    EnableWindow(context_.hwnd, TRUE);
    SetWindowPos(context_.hwnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_SHOWWINDOW | SWP_NOSIZE | SWP_NOMOVE);
    SetWindowPos(context_.hwnd, HWND_NOTOPMOST, 0, 0, 0, 0, SWP_SHOWWINDOW | SWP_NOSIZE | SWP_NOMOVE);
#endif
}

void window::disable()
{
    enabled_ = false;

#ifdef _WIN32
    EnableWindow(context_.hwnd, FALSE);
#endif
}

bool window::enabled() const
{
    return enabled_;
}

void window::switch_theme()
{
    if (control_callback)
    {
        std::string tooltip_text;
        bool continue_ = true;
        control_callback(window_control::theme, tooltip_text, continue_);
        switch_theme_button->set_caption(tooltip_text);
    }
}

void window::pin()
{
    if (active_control)
    {
        mouse_event me{ mouse_event_type::leave };
        send_event_to_control(active_control, { event_type::mouse, me });
        active_control.reset();
    }

    if (control_callback)
    {
        std::string tooltip_text;
        bool continue_ = true;
        control_callback(window_control::pin, tooltip_text, continue_);
        pin_button->set_caption(tooltip_text);
    }
}

void window::minimize()
{
    if (window_state_ == window_state::minimized)
    {
        return;
    }

    if (control_callback)
    {
        std::string text = "minimize";
        bool continue_ = true;
        control_callback(window_control::state, text, continue_);
        if (!continue_)
        {
            return;
        }
    }

    normal_position = position();

    prev_window_state_ = window_state_;

#ifdef _WIN32
    ShowWindow(context_.hwnd, SW_MINIMIZE);
#elif __linux__
    if (context_.connection)
    {
        xcb_client_message_event_t event = { 0 };

        event.window = context_.wnd;
        event.response_type = XCB_CLIENT_MESSAGE;
        event.type = wm_change_state;
        event.format = 32;
        event.data.data32[0] = XCB_ICCCM_WM_STATE_ICONIC;

        xcb_send_event(context_.connection, false, context_.screen->root, XCB_EVENT_MASK_SUBSTRUCTURE_REDIRECT | XCB_EVENT_MASK_SUBSTRUCTURE_NOTIFY, (const char*)&event);
        xcb_flush(context_.connection);
    }
#endif

    window_state_ = window_state::minimized;

    send_internal(internal_event_type::window_minimized, -1, -1);
}

void window::expand()
{
    if (window_state_ == window_state::maximized)
    {
        return;
    }

    if (control_callback)
    {
        std::string text = "expand";
        bool continue_ = true;
        control_callback(window_control::state, text, continue_);
        if (!continue_)
        {
            return;
        }
    }

    window_state_ = window_state::maximized;
    normal_position = position();

#ifdef _WIN32
    MONITORINFO mi = { sizeof(mi) };
    if (GetMonitorInfo(MonitorFromWindow(context_.hwnd, MONITOR_DEFAULTTOPRIMARY), &mi))
    {
        if (flag_is_set(window_style_, window_style::title_showed) && flag_is_set<DWORD>(mi.dwFlags, MONITORINFOF_PRIMARY)) // normal window maximization
        {
            RECT work_area;
            SystemParametersInfo(SPI_GETWORKAREA, 0, &work_area, 0);
            SetWindowPos(context_.hwnd, NULL, work_area.left, work_area.top, work_area.right, work_area.bottom, NULL);
        }
        else // expand to full screen
        {
            SetWindowPos(context_.hwnd,
                HWND_TOP,
                mi.rcMonitor.left, mi.rcMonitor.top,
                mi.rcMonitor.right - mi.rcMonitor.left,
                mi.rcMonitor.bottom - mi.rcMonitor.top,
                SWP_NOOWNERZORDER | SWP_FRAMECHANGED);
        }
    }
#elif __linux__
    if (!context_.connection)
    {
        return;
    }

    if (flag_is_set(window_style_, window_style::title_showed)) // normal window maximization
    {
        xcb_ewmh_connection_t ewmh_connection;

        if (xcb_ewmh_init_atoms_replies(&ewmh_connection,
            xcb_ewmh_init_atoms(context_.connection, &ewmh_connection),
            nullptr))
        {
            xcb_ewmh_get_workarea_reply_t wa;
            auto ok = xcb_ewmh_get_workarea_reply(&ewmh_connection,
                xcb_ewmh_get_workarea(&ewmh_connection, 0),
                &wa,
                nullptr);

            if (ok)
            {
                uint32_t values[] = { wa.workarea->x, wa.workarea->y, wa.workarea->width, wa.workarea->height };
                xcb_configure_window(context_.connection, context_.wnd, XCB_CONFIG_WINDOW_X | XCB_CONFIG_WINDOW_Y |
                    XCB_CONFIG_WINDOW_WIDTH | XCB_CONFIG_WINDOW_HEIGHT, values);

                xcb_flush(context_.connection);
            }

            xcb_ewmh_get_workarea_reply_wipe(&wa);
            xcb_ewmh_connection_wipe(&ewmh_connection);
        }
    }
    else // fullscreen
    {
        uint32_t values[] = { 0, 0, context_.screen->width_in_pixels, context_.screen->height_in_pixels };
        xcb_configure_window(context_.connection, context_.wnd, XCB_CONFIG_WINDOW_X | XCB_CONFIG_WINDOW_Y |
            XCB_CONFIG_WINDOW_WIDTH | XCB_CONFIG_WINDOW_HEIGHT, values);

        xcb_flush(context_.connection);
    }
#endif
    expand_button->set_image(theme_image(ti_normal, theme_));
}

void window::normal()
{
    if (control_callback)
    {
        std::string text = "normal";
        bool continue_ = true;
        control_callback(window_control::state, text, continue_);
        if (!continue_)
        {
            return;
        }
    }

#ifdef _WIN32
    SetWindowPos(context_.hwnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_SHOWWINDOW | SWP_NOSIZE | SWP_NOMOVE);
    SetWindowPos(context_.hwnd, HWND_NOTOPMOST, 0, 0, 0, 0, SWP_SHOWWINDOW | SWP_NOSIZE | SWP_NOMOVE);
    
    ShowWindow(context_.hwnd, SW_RESTORE);
#elif __linux__
    if (context_.connection)
    {
        xcb_client_message_event_t event = { 0 };

        event.window = context_.wnd;
        event.response_type = XCB_CLIENT_MESSAGE;
        event.type = wm_change_state;
        event.format = 32;
        event.data.data32[0] = XCB_ICCCM_WM_STATE_NORMAL;

        xcb_send_event(context_.connection, false, context_.screen->root, XCB_EVENT_MASK_SUBSTRUCTURE_REDIRECT | XCB_EVENT_MASK_SUBSTRUCTURE_NOTIFY, (const char*)&event);
        xcb_flush(context_.connection);

        /// todo bring window to top
    }
#endif

    window_state_ = window_state::normal;

    if (!normal_position.is_null())
    {
        set_position(normal_position, false);
    }

    expand_button->set_image(theme_image(ti_expand, theme_));

    update_buttons();

    send_internal(internal_event_type::window_normalized, position_.width(), position_.height()); 

    redraw({ 0, 0, position_.width(), position_.height() }, true);
}

window_state window::state() const
{
    return window_state_;
}

void window::set_caption(const std::string &caption_)
{
    caption = caption_;

    if (flag_is_set(window_style_, window_style::title_showed) && !parent_.lock())
    {
#ifdef _WIN32
        SetWindowText(context_.hwnd, boost::nowide::widen(caption).c_str());
#elif __linux__
        if (context_.connection)
        {
            xcb_icccm_set_wm_name(context_.connection, context_.wnd,
                XCB_ATOM_STRING, 8,
                caption.size(), caption.c_str());

            xcb_icccm_set_wm_icon_name(context_.connection, context_.wnd,
                XCB_ATOM_STRING, 8,
                caption.size(), caption.c_str());

            std::string class_hint = caption + '\0' + caption + '\0';
            xcb_icccm_set_wm_class(context_.connection, context_.wnd, class_hint.size(), class_hint.c_str());
        }
#endif
        redraw({ 0, 0, position_.width(), 30 });
    }
}

void window::set_style(window_style style)
{
    window_style_ = style;

    update_buttons();

#ifdef _WIN32
    if (topmost())
    {
        SetWindowPos(context_.hwnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
        if (window_state_ == window_state::maximized)
        {
            MONITORINFO mi = { sizeof(mi) };
            if (GetMonitorInfo(MonitorFromWindow(context_.hwnd, MONITOR_DEFAULTTOPRIMARY), &mi))
            {
                SetWindowPos(context_.hwnd,
                    HWND_TOP,
                    mi.rcMonitor.left, mi.rcMonitor.top,
                    mi.rcMonitor.right - mi.rcMonitor.left,
                    mi.rcMonitor.bottom - mi.rcMonitor.top,
                    SWP_NOOWNERZORDER | SWP_FRAMECHANGED);
            }
        }
    }
    else
    {
        SetWindowPos(context_.hwnd, HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
    }
#elif __linux__
    update_window_style();

    redraw({ 0, 0, position_.width(), 30 }, true);
#endif
}

void window::set_min_size(int32_t width, int32_t height)
{
    min_width = width;
    min_height = height;
}

void window::set_transient_for(std::shared_ptr<window> window_, bool docked__)
{
    transient_window = window_;
    docked_ = docked__;
}

void window::start_docking(std::shared_ptr<i_control> control)
{
    enabled_ = false;

    docked_control = control;

    send_internal(internal_event_type::set_focus, 0, 0);

    set_focused(control);
}

void window::end_docking()
{
    enabled_ = true;

    docked_control.reset();
}

void window::emit_event(int32_t x, int32_t y)
{
    auto parent__ = parent_.lock();
    if (!parent__)
    {
#ifdef _WIN32
        PostMessage(context_.hwnd, WM_USER, x, y);
#elif __linux__
        if (context_.connection)
        {
            xcb_client_message_event_t event = { 0 };

            event.window = context_.wnd;
            event.response_type = XCB_CLIENT_MESSAGE;
            event.type = XCB_ATOM_WM_COMMAND;
            event.format = 32;
            event.data.data32[0] = 0;
            event.data.data32[1] = x;
            event.data.data32[2] = y;

            xcb_send_event(context_.connection, false, context_.wnd, XCB_EVENT_MASK_NO_EVENT, (const char*)&event);
            xcb_flush(context_.connection);
        }
#endif
    }
    else
    {
        parent__->emit_event(x, y);
    }
}

void window::set_control_callback(std::function<void(window_control control, std::string &text, bool &continue_)> callback_)
{
    control_callback = callback_;
}

void window::set_default_push_control(std::shared_ptr<i_control> control)
{
    default_push_control = control;
}

void window::send_event_to_control(const std::shared_ptr<i_control> &control_, const event &ev)
{
    auto it = std::find_if(subscribers_.begin(), subscribers_.end(), [control_, ev](const event_subscriber &es) {
        return flag_is_set(es.event_types, ev.type) && es.control == control_;
    });
    
    if (it != subscribers_.end())
    {
        it->receive_callback(ev);
    }
}

void window::send_event_to_plains(const event &ev)
{
    auto subscribers__ = subscribers_; // This is necessary to be able to remove the subscriber in the callback
    for (auto &s : subscribers__)
    {
        if (!s.control && flag_is_set(s.event_types, ev.type) && s.receive_callback)
        {
            s.receive_callback(ev);
        }
    }
}

void window::send_mouse_event(const mouse_event &ev)
{
    if (!enabled_ && !docked_control)
    {
        return;
    }

    if (active_control && !active_control->position().in(ev.x, ev.y))
    {
        mouse_event me{ mouse_event_type::leave };
        send_event_to_control(active_control, { event_type::mouse, me });

        active_control.reset();
    }

    send_event_to_plains({ event_type::mouse, ev });

    auto send_mouse_event_to_control = [this](std::shared_ptr<wui::i_control> &send_to_control,
        const mouse_event &ev_) noexcept -> void
    {
        if (active_control == send_to_control)
        {
            if (send_to_control->focusing() && ev_.type == mouse_event_type::left_down)
            {
                set_focused(send_to_control);
            }

            return send_event_to_control(send_to_control, { event_type::mouse, ev_ });
        }
        else
        {
            if (active_control)
            {
                mouse_event me{ mouse_event_type::leave };
                send_event_to_control(active_control, { event_type::mouse, me });
            }

            if (ev_.y < 5 || (ev_.y < 24 && ev_.x > position().width() - 5)) /// control buttons border
            {
                return active_control.reset();
            }
            else
            {
                active_control = send_to_control;

                mouse_event me{ mouse_event_type::enter };
                return send_event_to_control(send_to_control, { event_type::mouse, me });
            }
        }
    };

    if (enabled_)
    {
        auto end = controls.rend();
        for (auto control = controls.rbegin(); control != end; ++control)
        {
            if (*control && (*control)->topmost() && (*control)->showed() && (*control)->position().in(ev.x, ev.y))
            {
                return send_mouse_event_to_control(*control, ev);
            }
        }

        for (auto control = controls.rbegin(); control != end; ++control)
        {
            if (*control && (*control)->showed() && (*control)->position().in(ev.x, ev.y))
            {
                return send_mouse_event_to_control(*control, ev);
            }
        }
    }
    else
    {
        for (auto &control : controls)
        {
            if (control && control->position().in(ev.x, ev.y) && control == docked_control)
            {
                return send_mouse_event_to_control(control, ev);
            }
        }
    }
}

bool window::check_control_here(int32_t x, int32_t y)
{
    for (auto &control : controls)
    {
        if (control->showed() &&
            control->position().in(x, y) &&
            std::find_if(subscribers_.begin(), subscribers_.end(), [&control](const event_subscriber &es) { return es.control == control; }) != subscribers_.end())
        {
            return true;
        }
    }
    return false;
}

void window::change_focus()
{
    if (controls.empty())
    {
        return;
    }

    for (auto &control : controls)
    {
        if (control->focused() && control != docked_control)
        {
            event ev;
            ev.type = event_type::internal;
            ev.internal_event_ = internal_event{ internal_event_type::remove_focus };
            send_event_to_control(control, ev);

            if (!control->focused()) /// need to change the focus inside the internal elements of the control
            {
                ++focused_index;
            }
            else
            {
                return;
            }
            break;
        }
    }

    size_t focusing_controls = 0;
    for (const auto &control : controls)
    {
        if (control->focusing())
        {
            ++focusing_controls;
        }
    }

    if (focused_index >= focusing_controls)
    {
        focused_index = 0;
    }

    set_focused(focused_index);
}

void window::execute_focused()
{
    std::shared_ptr<wui::i_control> control;

    if (docked_control)
    {
        control = docked_control;
    }
    else if (default_push_control)
    {
        control = default_push_control;
    }
    else
    {
        control = get_focused();
    }

    if (control)
    {
        event ev;
        ev.type = event_type::internal;
        ev.internal_event_ = internal_event{ internal_event_type::execute_focused };

        send_event_to_control(control, ev);
    }
}

void window::set_focused(std::shared_ptr<i_control> control)
{
    size_t index = 0;
    for (auto &c : controls)
    {
        if (c == control)
        {
            if (c->focused())
            {
                return;
            }
            focused_index = index;
        }

        if (c->focused())
        {
            event ev;
            ev.type = event_type::internal;
            ev.internal_event_ = internal_event{ internal_event_type::remove_focus };
            send_event_to_control(c, ev);
        }

        if (c->focusing())
        {
            ++index;
        }
    }

    if (control)
    {
        event ev;
        ev.type = event_type::internal;
        ev.internal_event_ = internal_event{ internal_event_type::set_focus };
        send_event_to_control(control, ev);
    }
}

void window::set_focused(size_t focused_index_)
{
    size_t index = 0;
    for (auto &control : controls)
    {
        if (control->focusing())
        {
            if (index == focused_index_)
            {
                event ev;
                ev.type = event_type::internal;
                ev.internal_event_ = internal_event{ internal_event_type::set_focus };
                send_event_to_control(control, ev);

                break;
            }

            ++index;
        }
    }
}

std::shared_ptr<i_control> window::get_focused()
{
    for (auto &control : controls)
    {
        if (control->focused())
        {
            return control;
        }
    }

    return nullptr;
}

void window::update_button_images()
{
    switch_theme_button->set_image(theme_image(ti_switch_theme, theme_));
    pin_button->set_image(theme_image(ti_pin, theme_));
    minimize_button->set_image(theme_image(ti_minimize, theme_));
    expand_button->set_image(theme_image(window_state_ == window_state::normal ? ti_expand : ti_normal, theme_));
    close_button->set_image(theme_image(ti_close, theme_));
}

void window::update_buttons()
{
	auto border_width = flag_is_set(window_style_, window_style::border_top) ? theme_dimension(tcn, tv_border_width, theme_) : 0;

    auto btn_size = 26;
    auto left = position_.width() - btn_size - 1;
    auto top = border_width;

    if (flag_is_set(window_style_, window_style::close_button))
    {
        close_button->set_position({ left, top, left + btn_size, top + btn_size }, false);
        close_button->show();

        left -= btn_size;
    }
    else
    {
        close_button->hide();
    }

    if (flag_is_set(window_style_, window_style::expand_button) || flag_is_set(window_style_, window_style::minimize_button))
    {
        expand_button->set_position({ left, top, left + btn_size, top + btn_size }, false);
        expand_button->show();

        left -= btn_size;

        if (flag_is_set(window_style_, window_style::expand_button))
        {
            expand_button->enable();
        }
        else
        {
            expand_button->disable();
        }
    }
    else
    {
        expand_button->hide();
    }

    if (flag_is_set(window_style_, window_style::minimize_button))
    {
        minimize_button->set_position({ left, top, left + btn_size, top + btn_size }, false);
        minimize_button->show();

        left -= btn_size;
    }
    else
    {
        minimize_button->hide();
    }

    if (flag_is_set(window_style_, window_style::pin_button))
    {
        pin_button->set_position({ left, top, left + btn_size, top + btn_size }, false);
        pin_button->show();

        left -= btn_size;
    }
    else
    {
        pin_button->hide();
    }

    if (flag_is_set(window_style_, window_style::switch_theme_button))
    {
        switch_theme_button->set_position({ left, top, left + btn_size, top + btn_size }, false);
        switch_theme_button->show();

        left -= btn_size;
    }
    else
    {
        switch_theme_button->hide();
    }
}

void window::draw_border(graphic &gr)
{
    auto c = theme_color(tcn, tv_border, theme_);
    auto x = theme_dimension(tcn, tv_border_width, theme_);

    int32_t l = 0, t = 0, w = 0, h = 0;

    auto pos = position();

    if (parent_.lock())
    {
        l = pos.left;
        t = pos.top;
        w = pos.right - x;
        h = pos.bottom - x;

    }
    else
    {
        w = position_.width() - x;
        h = position_.height() - x;
    }

    if (flag_is_set(window_style_, window_style::border_left))
    {
        gr.draw_line({ l, t, l, h }, c, x);
    }
    if (flag_is_set(window_style_, window_style::border_top))
    {
        gr.draw_line({ l, t, w, t }, c, x);
    }
    if (flag_is_set(window_style_, window_style::border_right))
    {
        gr.draw_line({ w, t, w, h }, c, x);
    }
    if (flag_is_set(window_style_, window_style::border_bottom))
    {
        gr.draw_line({ l, h, w, h }, c, x);
    }
}

void window::send_internal(internal_event_type type, int32_t x, int32_t y)
{
    event ev_;
    ev_.type = event_type::internal;
    ev_.internal_event_ = internal_event{ type, x, y };
    send_event_to_plains(ev_);
}

void window::send_system(system_event_type type, int32_t x, int32_t y)
{
    event ev_;
    ev_.type = event_type::system;
    ev_.system_event_ = system_event{ type, x, y };
    send_event_to_plains(ev_);
}

std::shared_ptr<window> window::get_transient_window()
{
    auto transient_window_ = transient_window.lock();

    while (transient_window_ && transient_window_->parent_.lock() != nullptr)
    {
        transient_window_ = transient_window_->parent_.lock();
    }

    return transient_window_;
}

bool window::init(const std::string &caption_, const rect &position__, window_style style, std::function<void(void)> close_callback_)
{
    caption = caption_;

    if (!position__.is_null())
    {
        position_ = position__;
    }

    window_style_ = style;
    close_callback = close_callback_;

    add_control(switch_theme_button, { 0 });
    add_control(pin_button, { 0 });
    add_control(minimize_button, { 0 });
    add_control(expand_button, { 0 });
    add_control(close_button, { 0 });

    update_button_images();
    update_buttons();

    auto transient_window_ = get_transient_window();
    if (transient_window_)
    {
        if (transient_window_->window_state_ == window_state::minimized)
        {
            transient_window_->normal();
        }

        if (docked_ && transient_window_->position_ > position_)
        {
			int32_t left = (transient_window_->position().width() - position_.width()) / 2;
            int32_t top = (transient_window_->position().height() - position_.height()) / 2;

			transient_window_->add_control(shared_from_this(), { left, top, left + position_.width(), top + position_.height() });
            transient_window_->start_docking(shared_from_this());
        }
        else
        {
            auto transient_window_pos = transient_window_->position();

            int32_t left = 0, top = 0;

            if (transient_window_pos.width() > 0 && transient_window_pos.height() > 0)
            {
                left = transient_window_pos.left + ((transient_window_pos.width() - position_.width()) / 2);
                top = transient_window_pos.top + ((transient_window_pos.height() - position_.height()) / 2);
            }
            else
            {
#ifdef _WIN32
                RECT work_area;
                SystemParametersInfo(SPI_GETWORKAREA, 0, &work_area, 0);
                auto screen_width = work_area.right - work_area.left,
                    screen_height = work_area.bottom - work_area.top;
                left = (screen_width - position_.width()) / 2;
                top = (screen_height - position_.height()) / 2;
#elif __linux__
                left = (context_.screen->width_in_pixels - position_.width()) / 2;
                top = (context_.screen->height_in_pixels - position_.height()) / 2;
#endif
            }

            position_.put(left, top);

            transient_window_->disable();
        }
    }

    auto parent__ = parent_.lock();
    if (parent__)
    {
        send_internal(internal_event_type::window_created, 0, 0);

        send_internal(internal_event_type::size_changed, position_.width(), position_.height());

        parent__->redraw(position());

        return true;
    }

#ifdef _WIN32
    auto h_inst = GetModuleHandle(NULL);

    WNDCLASSEXW wcex = { 0 };

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style = CS_DBLCLKS;
    wcex.lpfnWndProc = window::wnd_proc;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = sizeof(*this);
    wcex.hInstance = h_inst;
    wcex.hbrBackground = NULL;
    wcex.lpszClassName = L"WUI Window";

    RegisterClassExW(&wcex);

    if (position_.left == -1)
    {
        CenterHorizontally(position_, context_);
    }
    if (position_.top == -1)
    {
        CenterVertically(position_, context_);
    }
    
    context_.hwnd = CreateWindowEx(!topmost() ? 0 : WS_EX_TOPMOST, wcex.lpszClassName, L"", WS_VISIBLE | WS_POPUP | (window_state_ == window_state::minimized ? WS_MINIMIZE : 0),
        position_.left, position_.top, position_.width(), position_.height(), nullptr, nullptr, h_inst, this);

    if (!context_.hwnd)
    {
        return false;
    }

    if (window_state_ == window_state::maximized)
    {
        expand();
    }

    SetWindowText(context_.hwnd, boost::nowide::widen(caption).c_str());

    UpdateWindow(context_.hwnd);

    send_internal(internal_event_type::size_changed, position_.width(), position_.height());

    if (!showed_)
    {
        ShowWindow(context_.hwnd, SW_HIDE);
    }

#elif __linux__

    context_.display = XOpenDisplay(NULL);
    if (!context_.display)
    {
        fprintf(stderr, "window can't open make the connection to X server\n");
        return false;
    }

    XSetEventQueueOwner(context_.display, XCBOwnsEventQueue);
    context_.connection = XGetXCBConnection(context_.display);

    init_atoms();

    context_.screen = xcb_setup_roots_iterator(xcb_get_setup(context_.connection)).data;

    if (position_.left == -1)
    {
        CenterHorizontally(position_, context_);
    }
    if (position_.top == -1)
    {
        CenterVertically(position_, context_);
    }

    context_.wnd = xcb_generate_id(context_.connection);

    uint32_t mask = XCB_CW_EVENT_MASK;
    uint32_t values[] = {
        XCB_EVENT_MASK_EXPOSURE       | XCB_EVENT_MASK_BUTTON_PRESS   |
        XCB_EVENT_MASK_BUTTON_RELEASE | XCB_EVENT_MASK_POINTER_MOTION |
        XCB_EVENT_MASK_ENTER_WINDOW   | XCB_EVENT_MASK_LEAVE_WINDOW   |
        XCB_EVENT_MASK_KEY_PRESS      | XCB_EVENT_MASK_KEY_RELEASE    |
        XCB_EVENT_MASK_STRUCTURE_NOTIFY | XCB_EVENT_MASK_PROPERTY_CHANGE };

    auto window_cookie = xcb_create_window(context_.connection,
                      XCB_COPY_FROM_PARENT,
                      context_.wnd,
                      context_.screen->root,
                      position_.left, position_.top,
                      position_.width(), position_.height(),
                      0,
                      XCB_WINDOW_CLASS_INPUT_OUTPUT,
                      context_.screen->root_visual,
                      mask, values);

    if (!check_cookie(window_cookie, context_.connection, "WUI error, can't create a new window\n"))
    {
        return false;
    }

    remove_window_decorations(context_);

    xcb_atom_t styles[2] = { 0 };
    uint32_t styles_count = 0;
    if (flag_is_set(window_style_, window_style::topmost))
    {
        styles[0] = net_wm_state_above;
        ++styles_count;
    }
    if (!showed_)
    {
        styles[1] = net_wm_state_skip_taskbar;
        ++styles_count;
        minimize();
    }
    xcb_change_property(context_.connection, XCB_PROP_MODE_REPLACE, context_.wnd,
        net_wm_state, XCB_ATOM_ATOM, 32, styles_count, styles);

    switch (window_state_)
    {
        case window_state::minimized:
        {
            xcb_client_message_event_t event = { 0 };

            event.window = context_.wnd;
            event.response_type = XCB_CLIENT_MESSAGE;
            event.type = wm_change_state;
            event.format = 32;
            event.data.data32[0] = XCB_ICCCM_WM_STATE_ICONIC;

            xcb_send_event(context_.connection, false, context_.screen->root, XCB_EVENT_MASK_SUBSTRUCTURE_REDIRECT | XCB_EVENT_MASK_SUBSTRUCTURE_NOTIFY, (const char*)&event);
            xcb_flush(context_.connection);
        }
        break;
        case window_state::maximized:
            expand();
        break;
    }

    if (transient_window_)
    {
        xcb_icccm_set_wm_transient_for(context_.connection, context_.wnd, transient_window_->context().wnd);
    }

    xcb_icccm_set_wm_name(context_.connection, context_.wnd,
        XCB_ATOM_STRING, 8,
        caption.size(), caption.c_str());

    xcb_icccm_set_wm_icon_name(context_.connection, context_.wnd,
        XCB_ATOM_STRING, 8,
        caption.size(), caption.c_str());

    std::string class_hint = caption + '\0' + caption + '\0';
    xcb_icccm_set_wm_class(context_.connection, context_.wnd, class_hint.size(), class_hint.c_str());

    xcb_change_property(context_.connection, XCB_PROP_MODE_REPLACE, context_.wnd, wm_protocols_event, 4, 32, 1, &wm_delete_msg);

    xcb_map_window(context_.connection, context_.wnd);

    xcb_flush(context_.connection);

    send_internal(internal_event_type::size_changed, position_.width(), position_.height());

    graphic_.init(get_screen_size(context_), theme_color(tcn, tv_background, theme_));
    graphic_.start_cairo_device(); /// this workaround is needed to prevent destruction in the depths of the cairo

    runned = true;
    if (thread.joinable()) thread.join();
    thread = std::thread(std::bind(&window::process_events, this));

    send_internal(internal_event_type::window_created, 0, 0);
#endif

    return true;
}

void window::destroy()
{
    if (control_callback)
    {
        std::string text = "close";
        bool continue_= true;
        control_callback(window_control::close, text, continue_);
        if (!continue_)
        {
            return;
        }
    }

    std::vector<std::shared_ptr<i_control>> controls_;
    controls_ = controls; /// This is necessary to solve the problem of removing child controls within a control

    for (auto &control : controls_)
    {
        if (control)
        {
            control->clear_parent();
        }
    }

    if (active_control)
    {
        mouse_event me{ mouse_event_type::leave };
        send_event_to_control(active_control, { event_type::mouse, me });
    }

    active_control.reset();

    controls.clear();

    auto parent__ = parent_.lock();
    if (parent__)
    {
        parent__->remove_control(shared_from_this());

        auto transient_window_ = get_transient_window();
        if (transient_window_)
        {
            transient_window_->end_docking();
        }

        if (close_callback)
        {
            close_callback();
        }
    }
    else
    {
#ifdef _WIN32
        DestroyWindow(context_.hwnd);
#elif __linux__
        send_destroy_event();
#endif
    }
}

/// Windows specified code
#ifdef _WIN32

uint8_t get_key_modifier()
{
    if (GetKeyState(VK_SHIFT) < 0)
    {
        return vk_shift;
    }
    else if (GetKeyState(VK_CAPITAL) & 0x0001)
    {
        return vk_capital;
    }
    else if (GetKeyState(VK_MENU) < 0)
    {
        return vk_alt;
    }
    else if (GetKeyState(VK_LCONTROL) < 0)
    {
        return vk_lcontrol;
    }
    else if (GetKeyState(VK_RCONTROL) < 0)
    {
        return vk_rcontrol;
    }
    else if (GetKeyState(VK_INSERT) & 0x0001)
    {
        return vk_insert;
    }
    return 0;
}

LRESULT CALLBACK window::wnd_proc(HWND hwnd, UINT message, WPARAM w_param, LPARAM l_param)
{
    switch (message)
    {
        case WM_CREATE:
        {
            SetWindowLongPtr(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(reinterpret_cast<CREATESTRUCT*>(l_param)->lpCreateParams));

            window* wnd = reinterpret_cast<window*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));

            wnd->graphic_.init(get_screen_size(wnd->context_), theme_color(wnd->tcn, tv_background, wnd->theme_));

            wnd->send_internal(internal_event_type::window_created, 0, 0);
        }
        break;
        case WM_PAINT:
        {
            window* wnd = reinterpret_cast<window*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));

            PAINTSTRUCT ps;
            auto bpdc = BeginPaint(hwnd, &ps);

            if (bpdc == NULL)
            {
                return 0;
            }

            const rect paint_rect{ ps.rcPaint.left,
                ps.rcPaint.top,
                ps.rcPaint.right,
                ps.rcPaint.bottom };

            if (ps.fErase)
            {
                wnd->graphic_.clear(paint_rect);
            }
            if (flag_is_set(wnd->window_style_, window_style::title_showed) && !wnd->parent_.lock())
            {
                auto caption_font = theme_font(wnd->tcn, tv_caption_font, wnd->theme_);

                auto caption_rect = wnd->graphic_.measure_text(wnd->caption, caption_font);
                caption_rect.move(5, 5);

                if (caption_rect.in(paint_rect))
                {
                    wnd->graphic_.draw_rect(caption_rect, theme_color(wnd->tcn, tv_background, wnd->theme_));
                    wnd->graphic_.draw_text(caption_rect,
                        wnd->caption,
                        theme_color(wnd->tcn, tv_text, wnd->theme_),
                        caption_font);
                }
            }

            wnd->draw_border(wnd->graphic_);

            std::vector<std::shared_ptr<i_control>> topmost_controls;

            for (auto &control : wnd->controls)
            {
                if (control->position().in(paint_rect))
                {
                    if (!control->topmost())
                    {
                        control->draw(wnd->graphic_, paint_rect);
                    }
                    else
                    {
                        topmost_controls.emplace_back(control);
                    }
                }
            }

            for (auto &control : topmost_controls)
            {
                control->draw(wnd->graphic_, paint_rect);
            }

            wnd->graphic_.flush(paint_rect);

            EndPaint(hwnd, &ps);
        }
        break;
        case WM_MOUSEMOVE:
        {
            window* wnd = reinterpret_cast<window*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));

            RECT window_rect;
            GetWindowRect(hwnd, &window_rect);

            int16_t x_mouse = GET_X_LPARAM(l_param);
            int16_t y_mouse = GET_Y_LPARAM(l_param);

            static bool cursor_size_view = false;

            if (flag_is_set(wnd->window_style_, window_style::resizable) && wnd->window_state_ == window_state::normal)
            {
                if ((x_mouse > window_rect.right - window_rect.left - 5 && y_mouse > window_rect.bottom - window_rect.top - 5) ||
                    (x_mouse < 5 && y_mouse < 5))
                {
                    set_cursor(wnd->context_, cursor::size_nwse);
                    cursor_size_view = true;
                }
                else if ((x_mouse > window_rect.right - window_rect.left - 5 && y_mouse < 5) ||
                    (x_mouse < 5 && y_mouse > window_rect.bottom - window_rect.top - 5))
                {
                    set_cursor(wnd->context_, cursor::size_nesw);
                    cursor_size_view = true;
                }
                else if (x_mouse > window_rect.right - window_rect.left - 5 || x_mouse < 5)
                {
                    set_cursor(wnd->context_, cursor::size_we);
                    cursor_size_view = true;
                }
                else if (y_mouse > window_rect.bottom - window_rect.top - 5 || y_mouse < 5)
                {
                    set_cursor(wnd->context_, cursor::size_ns);
                    cursor_size_view = true;
                }
                else if (cursor_size_view &&
                    x_mouse > 5 && x_mouse < window_rect.right - window_rect.left - 5 &&
                    y_mouse > 5 && y_mouse < window_rect.bottom - window_rect.top - 5)
                {
                    set_cursor(wnd->context_, cursor::default_);
                    cursor_size_view = false;
                }
            }

            if (!wnd->mouse_tracked)
            {
                TRACKMOUSEEVENT track_mouse_event;

                track_mouse_event.cbSize = sizeof(track_mouse_event);
                track_mouse_event.dwFlags = TME_LEAVE;
                track_mouse_event.hwndTrack = hwnd;

                TrackMouseEvent(&track_mouse_event);

                wnd->mouse_tracked = true;
            }

            if (wnd->moving_mode_ != moving_mode::none)
            {
                switch (wnd->moving_mode_)
                {
                    case moving_mode::move:
                    {
                        int32_t x_window = window_rect.left + x_mouse - wnd->x_click;
                        int32_t y_window = window_rect.top + y_mouse - wnd->y_click;

                        SetWindowPos(hwnd, NULL, x_window, y_window, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
                    }
                    break;
                    case moving_mode::size_we_left:
                    {
                        POINT scr_mouse = { 0 };
                        GetCursorPos(&scr_mouse);

                        int32_t width = window_rect.right - window_rect.left - x_mouse;
                        int32_t height = window_rect.bottom - window_rect.top;

                        if (width > wnd->min_width && height > wnd->min_height)
                        {
                            SetWindowPos(hwnd, NULL, scr_mouse.x, window_rect.top, width, height, SWP_NOZORDER);
                        }
                    }
                    break;
                    case moving_mode::size_we_right:
                    {
                        int32_t width = x_mouse;
                        int32_t height = window_rect.bottom - window_rect.top;
                        if (width > wnd->min_width && height > wnd->min_height)
                        {
                            SetWindowPos(hwnd, NULL, 0, 0, width, height, SWP_NOMOVE | SWP_NOZORDER);
                        }
                    }
                    break;
                    case moving_mode::size_ns_top:
                    {
                        POINT scr_mouse = { 0 };
                        GetCursorPos(&scr_mouse);

                        int32_t width = window_rect.right - window_rect.left;
                        int32_t height = window_rect.bottom - window_rect.top - y_mouse;
                        if (width > wnd->min_width && height > wnd->min_height)
                        {
                            SetWindowPos(hwnd, NULL, window_rect.left, scr_mouse.y, width, height, SWP_NOZORDER);
                        }
                    }
                    break;
                    case moving_mode::size_ns_bottom:
                    {
                        int32_t width = window_rect.right - window_rect.left;
                        int32_t height = y_mouse;
                        if (width > wnd->min_width && height > wnd->min_height)
                        {
                            SetWindowPos(hwnd, NULL, 0, 0, width, height, SWP_NOMOVE | SWP_NOZORDER);
                        }
                    }
                    break;
                    case moving_mode::size_nesw_top:
                    {
                        POINT scr_mouse = { 0 };
                        GetCursorPos(&scr_mouse);

                        int32_t width = x_mouse;
                        int32_t height = window_rect.bottom - window_rect.top - y_mouse;
                        if (width > wnd->min_width && height > wnd->min_height)
                        {
                            SetWindowPos(hwnd, NULL, window_rect.left, scr_mouse.y, width, height, SWP_NOZORDER);
                        }
                    }
                    break;
                    case moving_mode::size_nwse_bottom:
                    {
                        int32_t width = x_mouse;
                        int32_t height = y_mouse;
                        if (width > wnd->min_width && height > wnd->min_height)
                        {
                            SetWindowPos(hwnd, NULL, 0, 0, width, height, SWP_NOMOVE | SWP_NOZORDER);
                        }
                    }
                    break;
                    case moving_mode::size_nwse_top:
                    {
                        POINT scrMouse = { 0 };
                        GetCursorPos(&scrMouse);

                        int32_t width = window_rect.right - window_rect.left - x_mouse;
                        int32_t height = window_rect.bottom - window_rect.top - y_mouse;
                        if (width > wnd->min_width && height > wnd->min_height)
                        {
                            SetWindowPos(hwnd, NULL, scrMouse.x, scrMouse.y, width, height, SWP_NOZORDER);
                        }
                    }
                    break;
                    case moving_mode::size_nesw_bottom:
                    {
                        POINT scr_mouse = { 0 };
                        GetCursorPos(&scr_mouse);

                        int32_t width = window_rect.right - window_rect.left - x_mouse;
                        int32_t height = y_mouse;
                        if (width > wnd->min_width && height > wnd->min_height)
                        {
                            SetWindowPos(hwnd, NULL, scr_mouse.x, window_rect.top, width, height, SWP_NOZORDER);
                        }
                    }
                    break;
                }
            }
            else
            {
                wnd->send_mouse_event({ mouse_event_type::move, x_mouse, y_mouse });
            }
        }
        break;
        case WM_LBUTTONDOWN:
        {
            window* wnd = reinterpret_cast<window*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));

            SetCapture(hwnd);

            RECT window_rect;
            GetWindowRect(hwnd, &window_rect);

            wnd->x_click = GET_X_LPARAM(l_param);
            wnd->y_click = GET_Y_LPARAM(l_param);

            wnd->send_mouse_event({ mouse_event_type::left_down, wnd->x_click, wnd->y_click });

            if (wnd->window_state_ == window_state::normal)
            {
                if (flag_is_set(wnd->window_style_, window_style::moving) &&
                    !wnd->check_control_here(wnd->x_click, wnd->y_click))
                {
                    wnd->moving_mode_ = moving_mode::move;
                }

                if (flag_is_set(wnd->window_style_, window_style::resizable))
                {
                    if (wnd->x_click > window_rect.right - window_rect.left - 5 && wnd->y_click > window_rect.bottom - window_rect.top - 5)
                    {
                        wnd->moving_mode_ = moving_mode::size_nwse_bottom;
                    }
                    else if (wnd->x_click < 5 && wnd->y_click < 5)
                    {
                        wnd->moving_mode_ = moving_mode::size_nwse_top;
                    }
                    else if (wnd->x_click > window_rect.right - window_rect.left - 5 && wnd->y_click < 5)
                    {
                        wnd->moving_mode_ = moving_mode::size_nesw_top;
                    }
                    else if (wnd->x_click < 5 && wnd->y_click > window_rect.bottom - window_rect.top - 5)
                    {
                        wnd->moving_mode_ = moving_mode::size_nesw_bottom;
                    }
                    else if (wnd->x_click > window_rect.right - window_rect.left - 5)
                    {
                        wnd->moving_mode_ = moving_mode::size_we_right;
                    }
                    else if (wnd->x_click < 5)
                    {
                        wnd->moving_mode_ = moving_mode::size_we_left;
                    }
                    else if (wnd->y_click > window_rect.bottom - window_rect.top - 5)
                    {
                        wnd->moving_mode_ = moving_mode::size_ns_bottom;
                    }
                    else if (wnd->y_click < 5)
                    {
                        wnd->moving_mode_ = moving_mode::size_ns_top;
                    }
                }
            }
        }
        break;
        case WM_LBUTTONUP:
        {
            ReleaseCapture();

            window* wnd = reinterpret_cast<window*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));

            wnd->moving_mode_ = moving_mode::none;

            wnd->send_mouse_event({ mouse_event_type::left_up, GET_X_LPARAM(l_param), GET_Y_LPARAM(l_param) });
        }
        break;
        case WM_RBUTTONDOWN:
        {
            window* wnd = reinterpret_cast<window*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));
            wnd->send_mouse_event({ mouse_event_type::right_down, GET_X_LPARAM(l_param), GET_Y_LPARAM(l_param) });
        }
        break;
        case WM_RBUTTONUP:
        {
            window* wnd = reinterpret_cast<window*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));
            wnd->send_mouse_event({ mouse_event_type::right_up, GET_X_LPARAM(l_param), GET_Y_LPARAM(l_param) });
        }
        break;
        case WM_LBUTTONDBLCLK:
        {
            ReleaseCapture();

            window* wnd = reinterpret_cast<window*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));

            wnd->moving_mode_ = moving_mode::none;

            wnd->send_mouse_event({ mouse_event_type::left_double, GET_X_LPARAM(l_param), GET_Y_LPARAM(l_param) });
        }
        break;
        case WM_MOUSELEAVE:
        {
            window* wnd = reinterpret_cast<window*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));
            wnd->mouse_tracked = false;
            wnd->send_mouse_event({ mouse_event_type::leave });
        }
        break;
        case WM_MOUSEWHEEL:
        {
            window* wnd = reinterpret_cast<window*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));
            POINT p = { GET_X_LPARAM(l_param), GET_Y_LPARAM(l_param) };
            ScreenToClient(hwnd, &p);
            wnd->send_mouse_event({ mouse_event_type::wheel, p.x, p.y, GET_WHEEL_DELTA_WPARAM(w_param) });
        }
        break;
        case WM_SIZE:
        {
            window* wnd = reinterpret_cast<window*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));

            auto width = LOWORD(l_param), height = HIWORD(l_param);

            wnd->position_ = { wnd->position_.left, wnd->position_.top, wnd->position_.left + width, wnd->position_.top + height };

            wnd->update_buttons();

            wnd->send_internal(wnd->window_state_ != window_state::maximized ? internal_event_type::size_changed : internal_event_type::window_expanded, width, height);

            RECT invalidatingRect = { 0, 0, width, height };
            InvalidateRect(hwnd, &invalidatingRect, FALSE);
        }
        break;
        case WM_MOVE:
        {
            window* wnd = reinterpret_cast<window*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));

            RECT window_rect = { 0 };
            GetWindowRect(hwnd, &window_rect);
            wnd->position_ = rect{ window_rect.left, window_rect.top, window_rect.right, window_rect.bottom };

            wnd->send_internal(internal_event_type::position_changed, window_rect.left, window_rect.top);
        }
        break;
        case WM_SYSCOMMAND:
            if (w_param == SC_RESTORE)
            {
                window* wnd = reinterpret_cast<window*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));
                wnd->window_state_ = wnd->prev_window_state_;
            }
            return DefWindowProc(hwnd, message, w_param, l_param);
        break;
        case WM_KEYDOWN:
        {
            window* wnd = reinterpret_cast<window*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));

            if (w_param == VK_TAB)
            {
                wnd->change_focus();
            }
            else if (w_param == VK_RETURN)
            {
                wnd->execute_focused();
            }

            event ev;
            ev.type = event_type::keyboard;
            ev.keyboard_event_ = keyboard_event{ keyboard_event_type::down, get_key_modifier(), 0 };
            ev.keyboard_event_.key[0] = static_cast<uint8_t>(w_param);

            auto control = wnd->get_focused();
            if (control)
            {
                wnd->send_event_to_control(control, ev);
            }
            wnd->send_event_to_plains(ev);
        }
        break;
        case WM_KEYUP:
        {
            window* wnd = reinterpret_cast<window*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));

            event ev;
            ev.type = event_type::keyboard;
            ev.keyboard_event_ = keyboard_event{ keyboard_event_type::up, get_key_modifier(), 0 };
            ev.keyboard_event_.key[0] = static_cast<uint8_t>(w_param);

            auto control = wnd->get_focused();
            if (control)
            {
                wnd->send_event_to_control(control, ev);
            }
            wnd->send_event_to_plains(ev);
        }
        break;
        case WM_CHAR:
            if (w_param != VK_ESCAPE && w_param != VK_BACK)
            {
                window* wnd = reinterpret_cast<window*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));

                event ev;
                ev.type = event_type::keyboard;
                ev.keyboard_event_ = keyboard_event{ keyboard_event_type::key, get_key_modifier(), 0 };
                auto narrow_str = boost::nowide::narrow(reinterpret_cast<const wchar_t*>(&w_param));
                memcpy(ev.keyboard_event_.key, narrow_str.c_str(), narrow_str.size());
                ev.keyboard_event_.key_size = static_cast<uint8_t>(narrow_str.size());

                auto control = wnd->get_focused();
                if (control)
                {
                    wnd->send_event_to_control(control, ev);
                }
                wnd->send_event_to_plains(ev);
            }
        break;
        case WM_USER:
            reinterpret_cast<window*>(GetWindowLongPtr(hwnd, GWLP_USERDATA))->send_internal(internal_event_type::user_emitted, static_cast<int32_t>(w_param), static_cast<int32_t>(l_param));
        break;
        case WM_DEVICECHANGE:
            reinterpret_cast<window*>(GetWindowLongPtr(hwnd, GWLP_USERDATA))->send_system(system_event_type::device_change, static_cast<int32_t>(w_param), static_cast<int32_t>(l_param));
        break;
        case WM_DESTROY:
        {
            window* wnd = reinterpret_cast<window*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));

            wnd->graphic_.release();

            auto transient_window_ = wnd->get_transient_window();
            if (transient_window_)
            {
                transient_window_->enable();
            }

            if (wnd->close_callback)
            {
                wnd->close_callback();
            }

            wnd->context_.hwnd = 0;
        }
        break;
        default:
            return DefWindowProc(hwnd, message, w_param, l_param);
    }
    return 0;
}

#elif __linux__

uint8_t normalize_modifier(int32_t state)
{
    if (state >= 8192)
    {
        return state - 8192;
    }

    return state;
}

void window::process_events()
{
    xcb_generic_event_t *e = nullptr;
    while (runned && (e = xcb_wait_for_event(context_.connection)))
    {
        switch (e->response_type & ~0x80)
        {
            case XCB_EXPOSE:
            {
                auto expose = (*(xcb_expose_event_t*)e);

                const rect paint_rect{ expose.x, expose.y, expose.x + expose.width, expose.y + expose.height };

                if (expose.pad0 != 0)
                {
                    graphic_.clear(paint_rect);
                }

                if (flag_is_set(window_style_, window_style::title_showed) && parent_.lock() == nullptr)
                {
                    auto caption_font = theme_font(tcn, tv_caption_font, theme_);

                    auto caption_rect = graphic_.measure_text(caption, caption_font);
                    caption_rect.move(5, 5);

                    if (caption_rect.in(paint_rect))
                    {
                        graphic_.draw_rect(caption_rect, theme_color(tcn, tv_background, theme_));
                        graphic_.draw_text(caption_rect,
                            caption,
                            theme_color(tcn, tv_text, theme_),
                            caption_font);
                    }
                }

                draw_border(graphic_);

                std::vector<std::shared_ptr<i_control>> topmost_controls;

                for (auto &control : controls)
                {
                    if (control->position().in(paint_rect))
                    {
                        if (!control->topmost())
                        {
                            control->draw(graphic_, paint_rect);
                        }
                        else
                        {
                            topmost_controls.emplace_back(control);
                        }
                    }
                }

                for (auto &control : topmost_controls)
                {
                    control->draw(graphic_, paint_rect);
                }

                graphic_.flush(paint_rect);
            }
            break;
            case XCB_MOTION_NOTIFY:
            {
                auto *ev = (xcb_motion_notify_event_t *)e;

                int16_t x_mouse = ev->event_x;
                int16_t y_mouse = ev->event_y;

                static bool cursor_size_view = false;

                auto ws = get_window_size(context_);

                if (flag_is_set(window_style_, window_style::resizable) && window_state_ == window_state::normal)
                {
                    if ((x_mouse > ws.width() - 5 && y_mouse > ws.height() - 5) ||
                        (x_mouse < 5 && y_mouse < 5))
                    {
                        set_cursor(context_, cursor::size_nwse);
                        cursor_size_view = true;
                    }
                    else if ((x_mouse > ws.width() - 5 && y_mouse < 5) ||
                        (x_mouse < 5 && y_mouse > ws.height() - 5))
                    {
                        set_cursor(context_, cursor::size_nesw);
                        cursor_size_view = true;
                    }
                    else if (x_mouse > ws.width() - 5 || x_mouse < 5)
                    {
                        set_cursor(context_, cursor::size_we);
                        cursor_size_view = true;
                    }
                    else if (y_mouse > ws.height() - 5 || y_mouse < 5)
                    {
                        set_cursor(context_, cursor::size_ns);
                        cursor_size_view = true;
                    }
                    else if (cursor_size_view &&
                        x_mouse > 5 && x_mouse < ws.width() - 5 &&
                        y_mouse > 5 && y_mouse < ws.height() - 5)
                    {
                        set_cursor(context_, cursor::default_);
                        cursor_size_view = false;
                    }
                }

                if (moving_mode_ != moving_mode::none)
                {
                    switch (moving_mode_)
                    {
                        case moving_mode::move:
                        {
                            uint32_t x_window = ev->root_x - x_click;
                            uint32_t y_window = ev->root_y - y_click;

                            uint32_t values[] = { x_window, y_window };
                            xcb_configure_window(context_.connection, context_.wnd, XCB_CONFIG_WINDOW_X | XCB_CONFIG_WINDOW_Y, values);
                        }
                        break;
                        case moving_mode::size_we_left:
                        {
                            uint32_t width = ws.width() - x_mouse;
                            uint32_t height = ws.height();
                            if (width > min_width && height > min_height)
                            {
                                uint32_t values[] = { static_cast<uint32_t>(ev->root_x), width };
                                xcb_configure_window(context_.connection, context_.wnd, XCB_CONFIG_WINDOW_X |
                                    XCB_CONFIG_WINDOW_WIDTH, values);
                            }
                        }
                        break;
                        case moving_mode::size_we_right:
                        {
                            uint32_t width = x_mouse;
                            uint32_t height = ws.height();
                            if (width > min_width && height > min_height)
                            {
                                uint32_t values[] = { width };
                                xcb_configure_window(context_.connection, context_.wnd, XCB_CONFIG_WINDOW_WIDTH, values);
                            }
                        }
                        break;
                        case moving_mode::size_ns_top:
                        {
                            uint32_t width = ws.width();
                            uint32_t height = ws.height() - y_mouse;
                            if (width > min_width && height > min_height)
                            {
                                uint32_t values[] = { static_cast<uint32_t>(ev->root_y), height };
                                xcb_configure_window(context_.connection, context_.wnd, XCB_CONFIG_WINDOW_Y | XCB_CONFIG_WINDOW_HEIGHT, values);
                            }
                        }
                        break;
                        case moving_mode::size_ns_bottom:
                        {
                            uint32_t width = ws.width();
                            uint32_t height = y_mouse;
                            if (width > min_width && height > min_height)
                            {
                                uint32_t values[] = { height };
                                xcb_configure_window(context_.connection, context_.wnd, XCB_CONFIG_WINDOW_HEIGHT, values);
                            }
                        }
                        break;
                        case moving_mode::size_nesw_top:
                        {
                            uint32_t width = x_mouse;
                            uint32_t height = ws.height() - y_mouse;
                            if (width > min_width && height > min_height)
                            {
                                uint32_t values[] = { static_cast<uint32_t>(ws.left), static_cast<uint32_t>(ev->root_y), width, height };
                                xcb_configure_window(context_.connection, context_.wnd, XCB_CONFIG_WINDOW_X | XCB_CONFIG_WINDOW_Y |
                                    XCB_CONFIG_WINDOW_WIDTH | XCB_CONFIG_WINDOW_HEIGHT, values);
                            }
                        }
                        break;
                        case moving_mode::size_nwse_bottom:
                        {
                            uint32_t width = x_mouse;
                            uint32_t height = y_mouse;
                            if (width > min_width && height > min_height)
                            {
                                uint32_t values[] = { width, height };
                                xcb_configure_window(context_.connection, context_.wnd, XCB_CONFIG_WINDOW_WIDTH | XCB_CONFIG_WINDOW_HEIGHT, values);
                            }
                        }
                        break;
                        case moving_mode::size_nwse_top:
                        {
                            uint32_t width = ws.width() - x_mouse;
                            uint32_t height = ws.height() - y_mouse;
                            if (width > min_width && height > min_height)
                            {
                                uint32_t values[] = { static_cast<uint32_t>(ev->root_x), static_cast<uint32_t>(ev->root_y), width, height };
                                xcb_configure_window(context_.connection, context_.wnd, XCB_CONFIG_WINDOW_X | XCB_CONFIG_WINDOW_Y |
                                    XCB_CONFIG_WINDOW_WIDTH | XCB_CONFIG_WINDOW_HEIGHT, values);
                            }
                        }
                        break;
                        case moving_mode::size_nesw_bottom:
                        {
                            uint32_t width = ws.width() - x_mouse;
                            uint32_t height = y_mouse;
                            if (width > min_width && height > min_height)
                            {
                                uint32_t values[] = { static_cast<uint32_t>(ev->root_x), width, height };
                                xcb_configure_window(context_.connection, context_.wnd, XCB_CONFIG_WINDOW_X |
                                    XCB_CONFIG_WINDOW_WIDTH | XCB_CONFIG_WINDOW_HEIGHT, values);
                            }
                        }
                        break;
                    }
                }
                else
                {
                    send_mouse_event({ mouse_event_type::move, x_mouse, y_mouse });
                }
            }
            break;
            case XCB_BUTTON_PRESS:
            {
                auto *ev = (xcb_button_press_event_t *)e;
                if (ev->detail == 1)
                {
                    if (ev->time - prev_button_click > 200)
                    {
                        x_click = ev->event_x;
                        y_click = ev->event_y;

                        auto ws = get_window_size(context_);

                        send_mouse_event({ mouse_event_type::left_down, x_click, y_click });

                        if (window_state_ == window_state::normal)
                        {
                            if (flag_is_set(window_style_, window_style::moving) &&
                                !check_control_here(x_click, y_click))
                            {
                                moving_mode_ = moving_mode::move;
                            }

                            if (flag_is_set(window_style_, window_style::resizable))
                            {
                                if (x_click > ws.width() - 5 && y_click > ws.height() - 5)
                                {
                                    moving_mode_ = moving_mode::size_nwse_bottom;
                                }
                                else if (x_click < 5 && y_click < 5)
                                {
                                    moving_mode_ = moving_mode::size_nwse_top;
                                }
                                else if (x_click > ws.width() - 5 && y_click < 5)
                                {
                                    moving_mode_ = moving_mode::size_nesw_top;
                                }
                                else if (x_click < 5 && y_click > ws.height() - 5)
                                {
                                    moving_mode_ = moving_mode::size_nesw_bottom;
                                }
                                else if (x_click > ws.width() - 5)
                                {
                                    moving_mode_ = moving_mode::size_we_right;
                                }
                                else if (x_click < 5)
                                {
                                    moving_mode_ = moving_mode::size_we_left;
                                }
                                else if (y_click > ws.height() - 5)
                                {
                                    moving_mode_ = moving_mode::size_ns_bottom;
                                }
                                else if (y_click < 5)
                                {
                                    moving_mode_ = moving_mode::size_ns_top;
                                }
                            }
                        }
                    }
                }
                else if (ev->detail == 3)
                {
                    send_mouse_event({ mouse_event_type::right_down, ev->event_x, ev->event_y });
                }
                else if (ev->detail == 4)
                {
                    send_mouse_event({ mouse_event_type::wheel, ev->event_x, ev->event_y, 1 });
                }
                else if (ev->detail == 5)
                {
                    send_mouse_event({ mouse_event_type::wheel, ev->event_x, ev->event_y, -1 });
                }
            }
            break;
            case XCB_BUTTON_RELEASE:
            {
                moving_mode_ = moving_mode::none;

                auto *ev = (xcb_button_press_event_t *)e;
                if (ev->detail == 1)
                {
                    send_mouse_event({ ev->time - prev_button_click > 300 ? mouse_event_type::left_up : mouse_event_type::left_double, ev->event_x, ev->event_y });

                    prev_button_click = ev->time;
                }
                else if (ev->detail == 3)
                {
                    send_mouse_event({ mouse_event_type::right_up, ev->event_x, ev->event_y });
                }
            }
            break;
            case XCB_LEAVE_NOTIFY:
            	send_mouse_event({ mouse_event_type::leave });
            break;
            case XCB_KEY_PRESS:
            {
                auto ev_ = *(xcb_key_press_event_t *)e;

                if (ev_.detail == vk_tab)
                {
                    change_focus();
                }
                else if (ev_.detail == vk_return)
                {
                    execute_focused();
                }
                else if (ev_.detail == vk_esc || ev_.detail == vk_back || ev_.detail == vk_del || ev_.detail == vk_end || ev_.detail == vk_home || ev_.detail == vk_page_up || ev_.detail == vk_page_down || ev_.detail == vk_left || ev_.detail == vk_right || ev_.detail == vk_up || ev_.detail == vk_down || ev_.detail == vk_shift)
                {
                    event ev;
                    ev.type = event_type::keyboard;
                    ev.keyboard_event_ = keyboard_event{ keyboard_event_type::down, normalize_modifier(ev_.state), 0 };
                    ev.keyboard_event_.key[0] = static_cast<uint8_t>(ev_.detail);

                    auto control = get_focused();
                    if (control)
                    {
                        send_event_to_control(control, ev);
                    }
                    send_event_to_plains(ev);
                }
                else
                {
                    event ev;
                    ev.type = event_type::keyboard;
                    ev.keyboard_event_ = keyboard_event{ keyboard_event_type::key, normalize_modifier(ev_.state), 0 };

                    XKeyPressedEvent keyev;
                    keyev.display = context_.display;
                    keyev.keycode = ev_.detail;
                    keyev.state = ev_.state;

                    ev.keyboard_event_.key_size = static_cast<uint8_t>(XLookupString(&keyev, ev.keyboard_event_.key, sizeof(ev.keyboard_event_.key), nullptr, nullptr));
                    if (ev.keyboard_event_.key_size)
                    {
                        auto control = get_focused();
                        if (control)
                        {
                            send_event_to_control(control, ev);
                        }
                        send_event_to_plains(ev);
                    }
                }
            }
            break;
            case XCB_KEY_RELEASE:
            {
                auto ev_ = *(xcb_key_press_event_t *)e;

                event ev;
                ev.type = event_type::keyboard;
                ev.keyboard_event_ = keyboard_event{ keyboard_event_type::up, normalize_modifier(ev_.state), 0 };
                ev.keyboard_event_.key[0] = static_cast<uint8_t>(ev_.detail);

                auto control = get_focused();
                if (control)
                {
                    send_event_to_control(control, ev);
                }
                send_event_to_plains(ev);
            }
            break;
            case XCB_CONFIGURE_NOTIFY:
            {
                auto ev = (*(xcb_configure_notify_event_t*)e);

                auto old_position = position_;

                if (ev.width > 0 && ev.height > 0)
                {
                    position_ = { ev.x, ev.y, ev.x + ev.width, ev.y + ev.height };

                    if (ev.width != old_position.width())
                    {
                        update_buttons();
                    }

                    if (ev.width != old_position.width() || ev.height != old_position.height())
                    {
                        graphic_.clear({ 0, 0, ev.width, ev.height });
                    }

                    if (window_state_ == window_state::maximized)
                    {
                        send_internal(internal_event_type::window_expanded, ev.width, ev.height);
                        continue;
                    }

                    if (ev.width != old_position.width() || ev.height != old_position.height())
                    {
                        send_internal(internal_event_type::size_changed, ev.width, ev.height);
                    }
                    else
                    {
                        send_internal(internal_event_type::position_changed, ev.x, ev.y);
                    }
                }
            }
            break;
            case XCB_PROPERTY_NOTIFY:
            {
                auto ev = (*(xcb_property_notify_event_t*)e);

                if (ev.atom == net_wm_state)
                {
                    auto get_prop_cookie = xcb_get_property (context_.connection,
                        0,
                        context_.wnd,
                        ev.atom,
                        XCB_GET_PROPERTY_TYPE_ANY,
                        0,
                        1);

                    auto property_reply = xcb_get_property_reply(context_.connection, get_prop_cookie, nullptr);

                    if (property_reply->type == XCB_ATOM_ATOM && xcb_get_property_value_length(property_reply) > 0)
                    {
                        auto val = (xcb_atom_t*)xcb_get_property_value(property_reply);

                        if (*val == net_wm_state_focused && window_state_ == window_state::minimized)
                        {
                            window_state_ = prev_window_state_;
                        }

                        free(property_reply);
                    }
                }
            }
            break;
            case XCB_CLIENT_MESSAGE:
                if ((*(xcb_client_message_event_t*)e).data.data32[0] != wm_delete_msg)
                {
                    send_internal(internal_event_type::user_emitted, static_cast<int32_t>((*(xcb_client_message_event_t*)e).data.data32[1]), static_cast<int32_t>((*(xcb_client_message_event_t*)e).data.data32[2]));
                }
                else
                {
                    graphic_.end_cairo_device(); /// this workaround is needed to prevent destruction in the depths of the cairo
                    graphic_.release();

                    xcb_destroy_window(context_.connection, context_.wnd);
                    XCloseDisplay(context_.display);

                    runned = false;

                    auto transient_window_ = get_transient_window();
                    if (transient_window_)
                    {
                        transient_window_->enable();
                    }

                    if (close_callback)
                    {
                        close_callback();
                    }

                    context_.wnd = 0;
                    context_.screen = nullptr;
                    context_.connection = nullptr;
                    context_.display = nullptr;
                }
            break;
        }
        free(e);
    }
}

void window::init_atoms()
{
    auto wm_protocols_event_reply = xcb_intern_atom_reply(context_.connection,
        xcb_intern_atom(context_.connection, 1, 12,"WM_PROTOCOLS"), 0);
    wm_protocols_event = wm_protocols_event_reply->atom;
    free(wm_protocols_event_reply);

    auto wm_delete_msg_reply = xcb_intern_atom_reply(context_.connection,
        xcb_intern_atom(context_.connection, 0, 16, "WM_DELETE_WINDOW"), NULL);
    wm_delete_msg = wm_delete_msg_reply->atom;
    free(wm_delete_msg_reply);

    auto wm_change_state_reply = xcb_intern_atom_reply(context_.connection,
        xcb_intern_atom(context_.connection, 0, 15, "WM_CHANGE_STATE"), NULL);
    wm_change_state = wm_change_state_reply->atom;
    free(wm_change_state_reply);

    auto net_wm_state_reply = xcb_intern_atom_reply(context_.connection,
        xcb_intern_atom(context_.connection, 0, 13, "_NET_WM_STATE"), NULL);
    net_wm_state = net_wm_state_reply->atom;
    free(net_wm_state_reply);

    auto net_wm_state_focused_reply = xcb_intern_atom_reply(context_.connection,
        xcb_intern_atom(context_.connection, 0, 21, "_NET_WM_STATE_FOCUSED"), NULL);
    net_wm_state_focused = net_wm_state_focused_reply->atom;
    free(net_wm_state_focused_reply);

    auto net_wm_state_above_reply = xcb_intern_atom_reply(context_.connection,
        xcb_intern_atom(context_.connection, 0, 19, "_NET_WM_STATE_ABOVE"), NULL);
    net_wm_state_above = net_wm_state_above_reply->atom;
    free(net_wm_state_above_reply);

    auto net_wm_state_skip_taskbar_reply = xcb_intern_atom_reply(context_.connection,
        xcb_intern_atom(context_.connection, 0, 26, "_NET_WM_STATE_SKIP_TASKBAR"), NULL);
    net_wm_state_skip_taskbar = net_wm_state_skip_taskbar_reply->atom;
    free(net_wm_state_skip_taskbar_reply);

    auto net_active_window_reply = xcb_intern_atom_reply(context_.connection,
        xcb_intern_atom(context_.connection, 0, 18, "_NET_ACTIVE_WINDOW"), NULL);
    net_active_window = net_active_window_reply->atom;
    free(net_active_window_reply);
}

void window::send_destroy_event()
{
    if (context_.connection)
    {
        xcb_client_message_event_t event = { 0 };

        event.window = context_.wnd;
        event.response_type = XCB_CLIENT_MESSAGE;
        event.type = XCB_ATOM_WM_COMMAND;
        event.format = 32;
        event.data.data32[0] = wm_delete_msg;

        xcb_send_event(context_.connection, false, context_.wnd, XCB_EVENT_MASK_NO_EVENT, (const char*)&event);
        xcb_flush(context_.connection);
    }
}

void window::update_window_style()
{
    if (!context_.valid())
    {
        return;
    }

    auto change_style = [this](xcb_atom_t type, xcb_atom_t action, xcb_atom_t style) noexcept -> void
    {
        xcb_client_message_event_t event = { 0 };

        event.window = context_.wnd;
        event.response_type = XCB_CLIENT_MESSAGE;
        event.type = type;
        event.format = 32;
        event.data.data32[0] = action;
        event.data.data32[1] = style;

        xcb_send_event(context_.connection, false, context_.screen->root, XCB_EVENT_MASK_SUBSTRUCTURE_REDIRECT | XCB_EVENT_MASK_SUBSTRUCTURE_NOTIFY, (const char*)&event);
        xcb_flush(context_.connection);
    };

    change_style(net_wm_state, flag_is_set(window_style_, window_style::topmost) ? 1 : 0, net_wm_state_above);
    change_style(net_wm_state, showed_ ? 0 : 1, net_wm_state_skip_taskbar);
    change_style(wm_change_state, showed_ ? XCB_ICCCM_WM_STATE_NORMAL : XCB_ICCCM_WM_STATE_ICONIC, 0);
    if (showed_)
    {
        change_style(net_active_window, 0, 0);
    }
}

#endif

}
