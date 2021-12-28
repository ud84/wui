//
// Copyright (c) 2021-2022 Anton Golovkov (udattsk at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/ud84/WUI
//

#pragma once

#include <wui/window/i_window.hpp>
#include <wui/common/rect.hpp>
#include <wui/control/i_control.hpp>
#include <wui/event/event.hpp>

#include <vector>
#include <memory>

#ifdef _WIN32
#include <windows.h>
#endif

namespace wui
{

enum class window_state
{
    normal,
    minimized,
    maximized
};

class button;

class window : public i_window, public i_control, public std::enable_shared_from_this<window>
{
public:
    window();
    ~window();

    /// IWindow
    virtual bool init(window_type type, const rect &position, const std::wstring &caption, std::function<void(void)> close_callback, std::shared_ptr<i_theme> theme_ = nullptr);
    virtual void destroy();

    virtual void add_control(std::shared_ptr<i_control> control, const rect &position);
    virtual void remove_control(std::shared_ptr<i_control> control);

    virtual void redraw(const rect &position, bool clear = false);

	/// IControl
    virtual void draw(graphic &gr);

    virtual void receive_event(const event &ev);

    virtual void set_position(const rect &position);
    virtual rect position() const;

    virtual void set_parent(std::shared_ptr<window> window_);
    virtual void clear_parent();

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

    /// Window state methods
    void minimize();
    void expand();
    void normal();
    window_state window_state() const;

    /// Show/hide window caption and button
    void show_title();
    void hide_title();

    /// Methods used to block the window while a modal dialog is displayed
    void block();
    void unlock();

    /// Callbacks
    void set_size_change_callback(std::function<void(int32_t, int32_t)> size_change_callback);

private:
    std::vector<std::shared_ptr<i_control>> controls;
    std::shared_ptr<i_control> active_control;

    window_type window_type_;
    rect position_, normal_position;
    std::wstring caption;
    wui::window_state window_state_;
    std::shared_ptr<i_theme> theme_;

    bool showed_, enabled_, title_showed;

    size_t focused_index;

    std::shared_ptr<window> parent;

    enum class moving_mode
    {
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

    std::function<void(void)> close_callback;
    std::function<void(int32_t, int32_t)> size_change_callback;

    std::shared_ptr<i_theme> buttons_theme, close_button_theme;
    std::shared_ptr<button> minimize_button, expand_button, close_button;

#ifdef _WIN32
    HWND hwnd;

    HBRUSH background_brush;
    HFONT font;

    int16_t x_click, y_click;
    bool mouse_tracked;

    static LRESULT CALLBACK wnd_proc(HWND hWnd, UINT message, WPARAM w_param, LPARAM l_param);

    void make_primitives();
    void destroy_primitives();

    void update_position();
#endif

    void send_mouse_event(const mouse_event &ev);
    void change_focus();
    void execute_focused();
    void set_focused(std::shared_ptr<i_control> &control);

    void update_control_buttons_theme();
};

}
