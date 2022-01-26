//
// Copyright (c) 2021-2022 Anton Golovkov (udattsk at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/ud84/wui
//

#pragma once

#include <wui/window/i_window.hpp>
#include <wui/system/system_context.hpp>
#include <wui/control/i_control.hpp>
#include <wui/graphic/graphic.hpp>
#include <wui/event/event.hpp>
#include <wui/common/rect.hpp>

#include <vector>
#include <memory>

#include <thread>

namespace wui
{

enum class window_state
{
    normal,
    minimized,
    maximized,
    pinned
};

class button;

class window : public i_window, public i_control, public std::enable_shared_from_this<window>
{
public:
    window();
    virtual ~window();

    /// IWindow
    virtual bool init(const std::string &caption, const rect &position, window_style style, std::function<void(void)> close_callback, std::shared_ptr<i_theme> theme_ = nullptr);
    virtual void destroy();

    virtual void add_control(std::shared_ptr<i_control> control, const rect &position);
    virtual void remove_control(std::shared_ptr<i_control> control);

    virtual void redraw(const rect &position, bool clear = false);

    virtual int32_t subscribe(std::function<void(const event&)> receive_callback, event_type event_types, std::shared_ptr<i_control> control = nullptr);
    virtual void unsubscribe(int32_t subscriber_id);

    virtual system_context &context();

	/// IControl
    virtual void draw(graphic &gr);

    virtual void set_position(const rect &position);
    virtual rect position() const;

    virtual void set_parent(std::shared_ptr<window> window_);
    virtual void clear_parent();

    virtual bool topmost() const;

    virtual void set_focus();
    virtual bool remove_focus();
    virtual bool focused() const;
    virtual bool focusing() const;

    virtual void update_theme(std::shared_ptr<i_theme> theme_ = nullptr);

    virtual void show();
    virtual void hide();
    virtual bool showed() const;

    virtual void enable();
    virtual void disable();
    virtual bool enabled() const;

    /// Window style methods
    void set_style(window_style style);
    void set_min_size(int32_t width, int32_t height);

    void set_transient_for(std::shared_ptr<window> window_, bool docked = true);
    void start_docking();
    void end_docking();

    /// Window state methods
    void pin();
    void minimize();
    void expand();
    void normal();
    window_state state() const;

    /// Callbacks
    void set_pin_callback(std::function<void(std::string &tooltip_text)> pin_callback);

public:
    /// Control name in theme
    static constexpr const char *tc = "window";

    /// Used theme values
    static constexpr const char *tv_background = "background";
    static constexpr const char *tv_border = "border";
    static constexpr const char *tv_border_width = "border_width";
    static constexpr const char *tv_text = "text";
    static constexpr const char *tv_active_button = "active_button";
    static constexpr const char *tv_caption_font = "caption_font";

    ///Used theme images
    static constexpr const char *ti_close = "window_close";
    static constexpr const char *ti_expand = "window_expand";
    static constexpr const char *ti_normal = "window_normal";
    static constexpr const char *ti_minimize = "window_minimize";
    static constexpr const char *ti_pin = "window_pin";

private:
    system_context context_;
    graphic graphic_;

    std::vector<std::shared_ptr<i_control>> controls;
    std::shared_ptr<i_control> active_control;

    std::string caption;
    rect position_, normal_position;
    int32_t min_width, min_height;
    window_style window_style_;
    wui::window_state window_state_, prev_window_state_;
    std::shared_ptr<i_theme> theme_;

    bool showed_, enabled_;

    size_t focused_index;

    std::weak_ptr<window> parent;
    int32_t my_control_sid, my_plain_sid;

    std::weak_ptr<window> transient_window;
    bool docked_;

    struct event_subscriber
    {
        std::function<void(const event&)> receive_callback;
        event_type event_types;
        std::shared_ptr<i_control> control;
    };
    std::vector<event_subscriber> subscribers_;

    enum class moving_mode
    {
        none,
        move,
        size_we_left,
        size_we_right,
        size_ns_top,
        size_ns_bottom,
        size_nwse_top,
        size_nwse_bottom,
        size_nesw_top,
        size_nesw_bottom
    };
    moving_mode moving_mode_;

    int16_t x_click, y_click;

    std::function<void(void)> close_callback;
    std::function<void(std::string &tooltip_text)> pin_callback;

    std::shared_ptr<i_theme> buttons_theme, close_button_theme;
    std::shared_ptr<button> pin_button, minimize_button, expand_button, close_button;

#ifdef _WIN32

    bool mouse_tracked;

    static LRESULT CALLBACK wnd_proc(HWND hWnd, UINT message, WPARAM w_param, LPARAM l_param);

#elif __linux__

    xcb_atom_t wm_protocols_event, wm_delete_msg, wm_change_state,
        net_wm_state, net_wm_state_focused, net_wm_state_above, net_wm_state_skip_taskbar, net_active_window;

    time_t prev_button_click;

    bool runned;
    std::thread thread;
    void process_events();

    void init_atoms();

    void send_destroy_event();

    void update_window_style();

#endif

    void set_position(const rect &position, bool change_value);

    void receive_event(const event &ev);

    bool send_event_to_control(std::shared_ptr<i_control> &control, const event &ev);
    void send_event_to_plains(const event &ev);
    bool send_mouse_event(const mouse_event &ev);

    void change_focus();
    void execute_focused();
    void set_focused(std::shared_ptr<i_control> &control);
    void set_focused(size_t index);
    std::shared_ptr<i_control> get_focused();

    void update_buttons(bool theme_changed);

    void draw_border(graphic &gr);
};

}
