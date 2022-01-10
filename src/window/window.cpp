﻿//
// Copyright (c) 2021-2022 Anton Golovkov (udattsk at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/ud84/WUI
//

#include <wui/window/window.hpp>

#include <wui/graphic/graphic.hpp>

#include <wui/theme/theme.hpp>

#include <wui/control/button.hpp>

#include <wui/common/flag_helpers.hpp>

#include <wui/system/tools.hpp>

#include <algorithm>

#ifdef _WIN32

#include <windowsx.h>
#include <resource.hpp>

#elif __linux__

#include <stdlib.h>
#include <xcb/xcb_atom.h>
#include <wui/common/char_helpers.hpp>

#endif

// Some helpers
#ifdef __linux__

void remove_window_decorations(wui::system_context &context)
{
    std::string mwh = "_MOTIF_WM_HINTS";
    xcb_intern_atom_reply_t *reply = xcb_intern_atom_reply(context.connection,
        xcb_intern_atom(context.connection, 0, mwh.size(), mwh.c_str()),
	    NULL);

    struct WMHints
    {
        uint32_t flags;
        uint32_t functions;
        uint32_t decorations;
        int32_t  input_mode;
        uint32_t status;
    };

    WMHints hints = { 0 };
    hints.flags = 2;

    xcb_change_property(context.connection,
        XCB_PROP_MODE_REPLACE,
		context.wnd,
        reply->atom,
		XCB_ATOM_WM_HINTS,
        32,
        5,
        &hints);

    free(reply);
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
	return wui::rect{ 0, 0, 0, 0 };
}

#endif

namespace wui
{

window::window()
    : context_{ 0 },
    graphic_(context_),
    controls(),
    active_control(),
    caption(),
    position_(), normal_position(),
    min_width(0), min_height(0),
    window_style_(window_style::frame),
    window_state_(window_state::normal),
    theme_(),
    showed_(true), enabled_(true),
    focused_index(0),
    parent(),
    moving_mode_(moving_mode::none),
    x_click(0), y_click(0),
    close_callback(),
    size_change_callback(),
    pin_callback(),
    buttons_theme(make_custom_theme()), close_button_theme(make_custom_theme()),
#ifdef _WIN32
    pin_button(new button(L"", std::bind(&window::pin, this), button_view::only_image, IDB_WINDOW_PIN, 24)),
    minimize_button(new button(L"", std::bind(&window::minimize, this), button_view::only_image, IDB_WINDOW_MINIMIZE, 24)),
    expand_button(new button(L"", [this]() { window_state_ == window_state::normal ? expand() : normal(); }, button_view::only_image, window_state_ == window_state::normal ? IDB_WINDOW_EXPAND : IDB_WINDOW_NORMAL, 24)),
    close_button(new button(L"", std::bind(&window::destroy, this), button_view::only_image, IDB_WINDOW_CLOSE, 24)),
    mouse_tracked(false)
#elif __linux__
    pin_button(new button(L"", std::bind(&window::pin, this), button_view::only_image, L"", 24)),
    minimize_button(new button(L"", std::bind(&window::minimize, this), button_view::only_image, L"", 24)),
    expand_button(new button(L"", [this]() { window_state_ == window_state::normal ? expand() : normal(); }, button_view::only_image, window_state_ == window_state::normal ? L"" : L"", 24)),
    close_button(new button(L"", std::bind(&window::destroy, this), button_view::only_image, L"", 24)),
	wm_protocols_event(nullptr), wm_delete_msg(nullptr),
	runned(false),
	thread()
#endif
{
    pin_button->disable_focusing();
    minimize_button->disable_focusing();
    expand_button->disable_focusing();
    close_button->disable_focusing();
}

window::~window()
{
	if (parent)
    {
        parent->remove_control(shared_from_this());
    }
#ifdef __linux__
    send_destroy_event();
    if (thread.joinable()) thread.join();
#endif
}

void window::add_control(std::shared_ptr<i_control> control, const rect &control_position)
{
    if (std::find(controls.begin(), controls.end(), control) == controls.end())
    {
        control->set_position(control_position);
        control->set_parent(shared_from_this());
        controls.emplace_back(control);

        redraw(control->position());
    }
}

void window::remove_control(std::shared_ptr<i_control> control)
{
    auto exist = std::find(controls.begin(), controls.end(), control);
    if (exist != controls.end())
    {
        (*exist)->clear_parent();
		
        redraw((*exist)->position(), true);

        controls.erase(exist);
    }
}

void window::redraw(const rect &redraw_position, bool clear)
{
    if (parent)
    {
        parent->redraw(redraw_position, clear);
    }
    else
    {
#ifdef _WIN32
        RECT invalidatingRect = { redraw_position.left, redraw_position.top, redraw_position.right, redraw_position.bottom };
        InvalidateRect(context_.hwnd, &invalidatingRect, clear ? TRUE : FALSE);
#elif __linux__
        if (context_.connection)
        {
        	if (!clear)
        	{
            	xcb_expose_event_t event = { 0 };

            	event.window = context_.wnd;
            	event.response_type = XCB_EXPOSE;
            	event.x = redraw_position.left;
            	event.y = redraw_position.top;
            	event.width = redraw_position.width();
            	event.height = redraw_position.height();

            	xcb_send_event(context_.connection, false, context_.wnd, XCB_EVENT_MASK_EXPOSURE, (const char*)&event);
            	xcb_flush(context_.connection);
        	}
        	else
            {
        		xcb_clear_area(context_.connection, 0, context_.wnd, redraw_position.left, redraw_position.top, redraw_position.width(), redraw_position.height());
            }
        }
#endif
    }
}

system_context &window::context()
{
	if (!parent)
	{
        return context_;
	}
	else
	{
        return parent->context();
	}
}

void window::draw(graphic &gr)
{
    if (!showed_)
    {
        return;
    }

    for (auto &control : controls)
    {
        control->draw(gr);
    }
}

void window::receive_event(const event &ev)
{
    if (!showed_)
    {
        return;
    }

    switch (ev.type)
    {
        case event_type::mouse:
            send_mouse_event(ev.mouse_event_);
        break;
        case event_type::internal:
            if (ev.internal_event_.type == internal_event_type::execute_focused)
            {
                execute_focused();
            }
        break;
    }
}

void window::set_position(const rect &position__)
{
    position_ = position__;
    normal_position = position_;

#ifdef _WIN32
    SetWindowPos(context_.hwnd, NULL, position_.left, position_.top, position_.right, position_.bottom, NULL);
#endif
}

rect window::position() const
{
    return position_;
}

void window::set_parent(std::shared_ptr<window> window)
{
    parent = window;

    if (parent)
    {
#ifdef _WIN32
        DestroyWindow(context_.hwnd);
        context_.hwnd = 0;
#elif __linux__
        send_destroy_event();
#endif

        for (auto &control : controls)
        {
            control->set_position({ control->position().left + position_.left,
                control->position().top + position_.top,
                control->position().right + position_.left,
                control->position().bottom + position_.top });
        }

        update_buttons(false);
    }
}

void window::clear_parent()
{
    if (parent)
    {
        for (auto &control : controls)
        {
            control->set_position({ control->position().left - position_.left,
                control->position().top - position_.top,
                control->position().right - position_.left,
                control->position().bottom - position_.top });
        }
    }

    parent.reset();
}

void window::set_focus()
{
    change_focus();
}

bool window::remove_focus()
{
    size_t focusing_controls = 0;
    for (const auto &control : controls)
    {
        if (control->focused())
        {
            control->remove_focus();
            ++focused_index;
        }

        if (control->focusing())
        {
            ++focusing_controls;
        }
    }

    if (focused_index >= focusing_controls)
    {
        focused_index = 0;
        return true;
    }

    controls[focused_index]->set_focus();

    return false;
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

void window::update_theme(std::shared_ptr<i_theme> theme__)
{
    if (theme_ && !theme__)
    {
        return;
    }
    theme_ = theme__;

#ifdef _WIN32

    if (!parent)
    {
        RECT client_rect;
        GetClientRect(context_.hwnd, &client_rect);
        InvalidateRect(context_.hwnd, &client_rect, TRUE);
    }
#elif __linux__
    if (!parent && context_.connection)
    {
        auto ws = get_window_size(context_);
        redraw(rect{ 0, 0, ws.width(), ws.height() }, true);
    }
#endif

    for (auto &control : controls)
    {
        control->update_theme(theme_);
    }

    update_buttons(true);
}

void window::show()
{
    showed_ = true;

    for (auto &control : controls)
    {
        control->show();
    }

#ifdef _WIN32
    if (!parent)
    {
        ShowWindow(context_.hwnd, SW_SHOW);
    }
#endif
}

void window::hide()
{
    showed_ = false;

    for (auto &control : controls)
    {
        control->hide();
    }

#ifdef _WIN32
    if (!parent)
    {
        ShowWindow(context_.hwnd, SW_HIDE);
    }
#endif
}

bool window::showed() const
{
    return showed_;
}

void window::enable()
{
    enabled_ = true;

    for (auto &control : controls)
    {
        control->enable();
    }
}

void window::disable()
{
    enabled_ = false;

    for (auto &control : controls)
    {
        control->disable();
    }
}

bool window::enabled() const
{
    return enabled_;
}

void window::pin()
{
    if (active_control)
    {
        mouse_event me{ mouse_event_type::leave, 0, 0 };
        active_control->receive_event({ event_type::mouse, me });
        active_control.reset();
    }

    if (pin_callback)
    {
        pin_callback();
    }
}

void window::minimize()
{
    if (window_state_ == window_state::minimized)
    {
        return;
    }

#ifdef _WIN32
    ShowWindow(context_.hwnd, SW_MINIMIZE);
#endif

    window_state_ = window_state::minimized;
}

void window::expand()
{
    window_state_ = window_state::maximized;

#ifdef _WIN32
    if (flag_is_set(window_style_, window_style::title_showed))
    {
        RECT work_area;
        SystemParametersInfo(SPI_GETWORKAREA, 0, &work_area, 0);

        SetWindowPos(context_.hwnd, NULL, work_area.left, work_area.top, work_area.right, work_area.bottom, NULL);
    }
    else
    {
        ShowWindow(context_.hwnd, SW_MAXIMIZE);
    }

    expand_button->set_image(IDB_WINDOW_NORMAL);
#endif
}

void window::normal()
{
    if (window_state_ == window_state::normal)
    {
        return;
    }

    set_position(normal_position);

    window_state_ = window_state::normal;

#ifdef _WIN32
    expand_button->set_image(IDB_WINDOW_EXPAND);
#endif
}

window_state window::state() const
{
    return window_state_;
}

void window::set_style(window_style style)
{
    window_style_ = style;

    update_buttons(false);

    redraw({ 0, 0, position_.width(), 30 }, false);
}

void window::set_min_size(int32_t width, int32_t height)
{
    min_width = width;
    min_height = height;
}

void window::block()
{
#ifdef _WIN32
    EnableWindow(context_.hwnd, FALSE);
#endif
}

void window::unlock()
{
#ifdef _WIN32
    EnableWindow(context_.hwnd, TRUE);
    SetWindowPos(context_.hwnd, HWND_TOP, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE);
#endif
}

void window::set_size_change_callback(std::function<void(int32_t, int32_t)> size_change_callback_)
{
    size_change_callback = size_change_callback_;
}

void window::set_pin_callback(std::function<void(void)> pin_callback_)
{
    pin_callback = pin_callback_;
}

bool window::send_mouse_event(const mouse_event &ev)
{
    if (active_control && !active_control->position().in(ev.x, ev.y))
    {
        mouse_event me{ mouse_event_type::leave, 0, 0 };
        active_control->receive_event({ event_type::mouse, me });

        active_control.reset();
    }

    auto end = controls.rend();
    for (auto control = controls.rbegin(); control != end; ++control)
    {
        if ((*control)->showed() && (*control)->position().in(ev.x, ev.y))
        {
            if (active_control == *control)
            {
                if (ev.type == mouse_event_type::left_up)
                {
                    set_focused(*control);
                }

                (*control)->receive_event({ event_type::mouse, ev });
            }
            else
            {
                if (active_control)
                {
                    mouse_event me{ mouse_event_type::leave, 0, 0 };
                    active_control->receive_event({ event_type::mouse, me });
                }

                active_control = *control;

                mouse_event me{ mouse_event_type::enter, 0, 0 };
                (*control)->receive_event({ event_type::mouse, me });
            }

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
        if (control->focused())
        {
            if (control->remove_focus()) // Need to change the focus inside the internal elements of the control
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

    size_t index = 0;
    for (auto &control : controls)
    {
        if (control->focusing())
        {
            if (index == focused_index)
            {
                control->set_focus();
                break;
            }
			
            ++index;
        }
    }
}

void window::execute_focused()
{
    auto control = get_focused();
    if (control)
    {
        event ev;
        ev.type = event_type::internal;
        ev.internal_event_ = internal_event{ internal_event_type::execute_focused };;

        control->receive_event(ev);
    }
}

void window::set_focused(std::shared_ptr<i_control> &control)
{
    size_t index = 0;
    for (auto &c : controls)
    {
        if (c == control && c->focused())
        {
            return;
        }

        if (c->focused())
        {
            c->remove_focus();
        }

        if (c == control)
        {
            focused_index = index;
        }

        ++index;
    }

    control->set_focus();
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

    return std::shared_ptr<i_control>();
}

void window::update_buttons(bool theme_changed)
{
    auto background_color = theme_color(theme_value::window_background, theme_);

    if (theme_changed)
    {
        buttons_theme->set_color(theme_value::button_calm, background_color);
        buttons_theme->set_color(theme_value::button_active, theme_color(theme_value::window_active_button, theme_));
        buttons_theme->set_color(theme_value::button_border, background_color);
        buttons_theme->set_color(theme_value::button_text, theme_color(theme_value::window_text, theme_));
        buttons_theme->set_color(theme_value::button_disabled, background_color);
        buttons_theme->set_dimension(theme_value::button_round, 0);
        buttons_theme->set_string(theme_value::images_path, theme_string(theme_value::images_path, theme_));

        pin_button->update_theme(buttons_theme);
        minimize_button->update_theme(buttons_theme);
        expand_button->update_theme(buttons_theme);
    
        close_button_theme->set_color(theme_value::button_calm, background_color);
        close_button_theme->set_color(theme_value::button_active, make_color(235, 15, 20));
        close_button_theme->set_color(theme_value::button_border, background_color);
        close_button_theme->set_color(theme_value::button_text, theme_color(theme_value::window_text, theme_));
        close_button_theme->set_color(theme_value::button_disabled, background_color);
        close_button_theme->set_dimension(theme_value::button_round, 0);
        close_button_theme->set_string(theme_value::images_path, theme_string(theme_value::images_path, theme_));

        close_button->update_theme(close_button_theme);
    }

    auto btn_size = 26;
    auto left = position_.right - btn_size;
    auto top = !parent ? 0 : position_.top;

    if (flag_is_set(window_style_, window_style::close_button))
    {
        close_button->set_position({ left, top, left + btn_size, top + btn_size });
        close_button->show();

        left -= btn_size;
    }
    else
    {
        close_button->hide();
    }

    if (flag_is_set(window_style_, window_style::expand_button) || flag_is_set(window_style_, window_style::minimize_button))
    {
        expand_button->set_position({ left, top, left + btn_size, top + btn_size });
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
        minimize_button->set_position({ left, top, left + btn_size, top + btn_size });
        minimize_button->show();

        left -= btn_size;
    }
    else
    {
        minimize_button->hide();
    }

    if (flag_is_set(window_style_, window_style::pin_button))
    {
        pin_button->set_position({ left, top, left + btn_size, top + btn_size });
        pin_button->show();
    }
    else
    {
        pin_button->hide();
    }
}

void window::update_position(const rect &new_position)
{   
    if (new_position.width() != 0 && new_position.height() != 0)
    {
        position_ = new_position;
        if (window_state_ != window_state::maximized)
        {
            normal_position = position_;
        }
    }
}

bool window::init(const std::wstring &caption_, const rect &position__, window_style style, std::function<void(void)> close_callback_, std::shared_ptr<i_theme> theme__)
{
    caption = caption_;
    position_ = position__;
    normal_position = position_;
    window_style_ = style;
    close_callback = close_callback_;
    theme_ = theme__;

    add_control(pin_button, { position_.right - 104, 0, position_.right - 78, 26 });
    add_control(minimize_button, { position_.right - 78, 0, position_.right - 52, 26 });
    add_control(expand_button, { position_.right - 52, 0, position_.right - 26, 26 });
    add_control(close_button, { position_.right - 26, 0, position_.right, 26 });

    update_buttons(true);

    if (parent)
    {
        showed_ = true;
        parent->redraw(position_);

        return true;
    }

#ifdef _WIN32
    auto h_inst = GetModuleHandle(NULL);

    WNDCLASSEXW wcex = { 0 };

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style = CS_DBLCLKS;
    wcex.lpfnWndProc = window::wnd_proc;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = sizeof(this);
    wcex.hInstance = h_inst;
    wcex.hbrBackground = NULL;
    wcex.lpszClassName = L"WUI Window";

    RegisterClassExW(&wcex);

    context_.hwnd = CreateWindowW(wcex.lpszClassName, L"", WS_VISIBLE | WS_POPUP,
        position_.left, position_.top, position_.right, position_.bottom, nullptr, nullptr, h_inst, this);

    if (!context_.hwnd)
    {
        return false;
    }

    SetWindowText(context_.hwnd, caption.c_str());

    UpdateWindow(context_.hwnd);

#elif __linux__

    context_.connection = xcb_connect(NULL, NULL);
    if (!context_.connection)
    {
    	fprintf(stderr, "window can't open make the connection to X server\n");
    	return false;
    }

    auto screen = xcb_setup_roots_iterator(xcb_get_setup(context_.connection)).data;

    context_.wnd = xcb_generate_id(context_.connection);

    uint32_t mask = XCB_CW_BACK_PIXEL | XCB_CW_EVENT_MASK;
    uint32_t values[] = { static_cast<uint32_t>(theme_color(theme_value::window_background, theme_)),
        XCB_EVENT_MASK_EXPOSURE       | XCB_EVENT_MASK_BUTTON_PRESS   |
        XCB_EVENT_MASK_BUTTON_RELEASE | XCB_EVENT_MASK_POINTER_MOTION |
        XCB_EVENT_MASK_ENTER_WINDOW   | XCB_EVENT_MASK_LEAVE_WINDOW   |
        XCB_EVENT_MASK_KEY_PRESS      | XCB_EVENT_MASK_KEY_RELEASE };

    xcb_create_window(context_.connection,
                      XCB_COPY_FROM_PARENT,
                      context_.wnd,
					  screen->root, // parent window
                      position_.left, position_.top,
                      position_.width(), position_.height(),
                      0,
                      XCB_WINDOW_CLASS_INPUT_OUTPUT,
					  screen->root_visual,
                      mask, values);

    remove_window_decorations(context_);

    xcb_change_property(context_.connection, XCB_PROP_MODE_REPLACE, context_.wnd,
        XCB_ATOM_WM_NAME, XCB_ATOM_STRING, 8, caption.size(), to_multibyte(caption).c_str());

    xcb_change_property(context_.connection, XCB_PROP_MODE_REPLACE, context_.wnd,
        XCB_ATOM_WM_ICON_NAME, XCB_ATOM_STRING, 8, caption.size(), to_multibyte(caption).c_str());

    wm_protocols_event = xcb_intern_atom_reply(context_.connection,
        xcb_intern_atom(context_.connection, 1, 12,"WM_PROTOCOLS"), 0);

    wm_delete_msg = xcb_intern_atom_reply(context_.connection,
        xcb_intern_atom(context_.connection, 0, 16, "WM_DELETE_WINDOW"), NULL);

    xcb_change_property(context_.connection, XCB_PROP_MODE_REPLACE, context_.wnd, (*wm_protocols_event).atom, 4, 32, 1, &(*wm_delete_msg).atom);

    xcb_map_window(context_.connection, context_.wnd);

    xcb_flush(context_.connection);

    runned = true;
    if (thread.joinable()) thread.join();
    thread = std::thread(std::bind(&window::process_events, this));
#endif

    return true;
}

void window::destroy()
{
    for (auto &control : controls)
    {
        control->clear_parent();
    }

    active_control.reset();
    controls.clear();

    if (parent)
    {
        parent->remove_control(shared_from_this());

        if (close_callback)
        {
            close_callback();
        }
    }
    else
    {
#ifdef _WIN32
        DestroyWindow(context_.hwnd);
        context_.hwnd = 0;
#elif __linux__
        send_destroy_event();
#endif
    }
}

/// Windows specified code
#ifdef _WIN32

wchar_t get_key_modifier()
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
        }
        break;
        case WM_PAINT:
        {
            window* wnd = reinterpret_cast<window*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));

            PAINTSTRUCT ps;
            wnd->context_.dc = BeginPaint(hwnd, &ps);

            RECT client_rect;
            GetClientRect(hwnd, &client_rect);
            wnd->graphic_.start_drawing(rect{ 0, 0, client_rect.right, client_rect.bottom }, theme_color(theme_value::window_background, wnd->theme_));

            if (flag_is_set(wnd->window_style_, window_style::title_showed))
            {
                wnd->graphic_.draw_text(rect{ 5, 5, 5, 5 },
                    wnd->caption,
                    theme_color(theme_value::window_text, wnd->theme_),
                    font_settings{ theme_string(theme_value::window_title_font_name, wnd->theme_),
                        theme_dimension(theme_value::window_title_font_size, wnd->theme_),
                        font_decorations::normal });
            }
		
            for (auto &control : wnd->controls)
            {
                control->draw(wnd->graphic_);
            }

            wnd->graphic_.end_drawing();

            EndPaint(wnd->context_.hwnd, &ps);
            wnd->context_.dc = 0;
        }
        break;
        case WM_MOUSEMOVE:
        {
            window* wnd = reinterpret_cast<window*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));

            RECT window_rect;
            GetWindowRect(hwnd, &window_rect);

            int16_t x_mouse = GET_X_LPARAM(l_param);
            int16_t y_mouse = GET_Y_LPARAM(l_param);

            if (flag_is_set(wnd->window_style_, window_style::resizable) && wnd->window_state_ == window_state::normal)
            {
                if ((x_mouse > window_rect.right - window_rect.left - 5 && y_mouse > window_rect.bottom - window_rect.top - 5) ||
                    (x_mouse < 5 && y_mouse < 5))
                {
                    set_cursor(context_, cursor::size_nwse);
                }
                else if ((x_mouse > window_rect.right - window_rect.left - 5 && y_mouse < 5) ||
                    (x_mouse < 5 && y_mouse > window_rect.bottom - window_rect.top - 5))
                {
                    set_cursor(context_, cursor::size_nesw);
                }
                else if (x_mouse > window_rect.right - window_rect.left - 5 || x_mouse < 5)
                {
                    set_cursor(context_, cursor::size_we);
                }
                else if (y_mouse > window_rect.bottom - window_rect.top - 5 || y_mouse < 5)
                {
                    set_cursor(context_, cursor::size_ns);
                }
                else if (!wnd->active_control)
                {
                	set_cursor(context_, cursor::default_);
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

            if (!wnd->send_mouse_event({ mouse_event_type::left_down, wnd->x_click, wnd->y_click }) && 
                (flag_is_set(wnd->window_style_, window_style::moving) && wnd->window_state_ == window_state::normal))
            {
                wnd->moving_mode_ = moving_mode::move;

                if (flag_is_set(wnd->window_style_, window_style::resizable) && wnd->window_state_ == window_state::normal)
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
            wnd->send_mouse_event({ mouse_event_type::leave, -1, -1 });
        }
        break;
        case WM_SIZE:
        {
            window* wnd = reinterpret_cast<window*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));

            auto width = LOWORD(l_param), height = HIWORD(l_param);

            wnd->update_position({ wnd->position_.left, wnd->position_.top, width, height });

            wnd->update_buttons(false);
			
            if (wnd->size_change_callback)
            {
                wnd->size_change_callback(LOWORD(l_param), HIWORD(l_param));
            }
        }
        break;
        case WM_MOVE:
        {
            window* wnd = reinterpret_cast<window*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));

            RECT window_rect = { 0 };
            GetWindowRect(hwnd, &window_rect);
            wnd->update_position({ window_rect.left, window_rect.top, window_rect.right - window_rect.left, window_rect.bottom - window_rect.top });
        }
        break;
        case WM_SYSCOMMAND:
            if (w_param == SC_RESTORE)
            {
                window* wnd = reinterpret_cast<window*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));
                wnd->window_state_ = window_state::normal;
            }
            return DefWindowProc(hwnd, message, w_param, l_param);
        break;
        case WM_KEYDOWN:
            switch (w_param)
            {
                case VK_TAB:
                {
                    window* wnd = reinterpret_cast<window*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));
                    wnd->change_focus();
                }
                break;
                case VK_RETURN:
                {
                    window* wnd = reinterpret_cast<window*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));
                    wnd->execute_focused();
                }
                break;
                case VK_BACK: case VK_DELETE: case VK_END: case VK_HOME: case VK_LEFT: case VK_RIGHT: case VK_SHIFT:
                {
                    window* wnd = reinterpret_cast<window*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));
                    auto control = wnd->get_focused();
                    if (control)
                    {
                        event ev;
                        ev.type = event_type::keyboard;
                        ev.keyboard_event_ = keyboard_event{ keyboard_event_type::down, get_key_modifier(), static_cast<wchar_t>(w_param) };;

                        control->receive_event(ev);
                    }
                }
                break;
            }
        break;
        case WM_KEYUP:
            switch (w_param)
            {
                case VK_BACK: case VK_DELETE: case VK_END: case VK_HOME: case VK_LEFT: case VK_RIGHT: case VK_SHIFT:
                {
                    window* wnd = reinterpret_cast<window*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));
                    auto control = wnd->get_focused();
                    if (control)
                    {
                        event ev;
                        ev.type = event_type::keyboard;
                        ev.keyboard_event_ = keyboard_event{ keyboard_event_type::up, get_key_modifier(), static_cast<wchar_t>(w_param) };;

                        control->receive_event(ev);
                    }
                }
                break;
            }
        break;
        case WM_CHAR:
            if (w_param != VK_BACK && w_param != VK_DELETE && w_param != VK_END && w_param != VK_HOME && w_param != VK_LEFT && w_param != VK_RIGHT && w_param != VK_SHIFT)
            {
                window* wnd = reinterpret_cast<window*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));
                auto control = wnd->get_focused();
                if (control)
                {
                    event ev;
                    ev.type = event_type::keyboard;
                    ev.keyboard_event_ = keyboard_event{ keyboard_event_type::key, get_key_modifier(), static_cast<wchar_t>(w_param) };;
                    
                    control->receive_event(ev);
                }
                break;
            }
        break;
        case WM_DESTROY:
        {
            window* wnd = reinterpret_cast<window*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));
            if (!wnd->parent && wnd->close_callback)
            {
                wnd->close_callback();
            }
        }
        break;
        default:
            return DefWindowProc(hwnd, message, w_param, l_param);
    }
    return 0;
}

#elif __linux__

void window::process_events()
{
	xcb_generic_event_t *e = nullptr;
    while (runned && (e = xcb_wait_for_event(context_.connection)))
    {
        switch (e->response_type & ~0x80)
        {
	        case XCB_EXPOSE:
	        {
	        	auto ws = get_window_size(context_);
	            graphic_.start_drawing(rect{ 0, 0, ws.width(), ws.height() }, theme_color(theme_value::window_background, theme_));

	            if (flag_is_set(window_style_, window_style::title_showed))
	            {
	                graphic_.draw_text(rect{ 5, 5, 5, 5 },
	                    caption,
	                    theme_color(theme_value::window_text, theme_),
	                    font_settings{ theme_string(theme_value::window_title_font_name, theme_),
	                        theme_dimension(theme_value::window_title_font_size, theme_),
	                        font_decorations::normal });
	            }

	            for (auto &control : controls)
	            {
	                control->draw(graphic_);
	            }

	            graphic_.end_drawing();

                xcb_flush(context_.connection);
            }
            break;
            case XCB_KEY_PRESS:
                destroy();
            break;
            case XCB_MOTION_NOTIFY:
            {
            	auto *ev = (xcb_motion_notify_event_t *)e;

            	int16_t x_mouse = ev->event_x;
                int16_t y_mouse = ev->event_y;

                auto ws = get_window_size(context_);

                if (flag_is_set(window_style_, window_style::resizable) && window_state_ == window_state::normal)
                {
                    if ((x_mouse > ws.width() - 5 && y_mouse > ws.height() - 5) ||
                        (x_mouse < 5 && y_mouse < 5))
                    {
                        set_cursor(context_, cursor::size_nwse);
                    }
                    else if ((x_mouse > ws.width() - 5 && y_mouse < 5) ||
                        (x_mouse < 5 && y_mouse > ws.height() - 5))
                    {
                    	set_cursor(context_, cursor::size_nesw);
                    }
                    else if (x_mouse > ws.width() - 5 || x_mouse < 5)
                    {
                    	set_cursor(context_, cursor::size_we);
                    }
                    else if (y_mouse > ws.height() - 5 || y_mouse < 5)
                    {
                    	set_cursor(context_, cursor::size_ns);
                    }
                    else if (!active_control)
                    {
                    	set_cursor(context_, cursor::default_);
                    }
                }

                if (moving_mode_ != moving_mode::none)
                {
                    switch (moving_mode_)
                    {
                        case moving_mode::move:
                        {
                            uint32_t x_window = ws.left + x_mouse - x_click;
                            uint32_t y_window = ws.top + y_mouse - y_click;

                            uint32_t values[] = { x_window, y_window };
                            xcb_configure_window(context_.connection, context_.wnd, XCB_CONFIG_WINDOW_X | XCB_CONFIG_WINDOW_Y, values);
                            xcb_flush(context_.connection);
                        }
                        break;
                        case moving_mode::size_we_left:
                        {
                            uint32_t width = ws.width() - x_mouse;
                            uint32_t height = ws.height();
                            if (width > min_width && height > min_height)
                            {
                                uint32_t values[] = { static_cast<uint32_t>(ev->root_x), static_cast<uint32_t>(ws.top), width, height };
                                xcb_configure_window(context_.connection, context_.wnd, XCB_CONFIG_WINDOW_X | XCB_CONFIG_WINDOW_Y |
                                    XCB_CONFIG_WINDOW_WIDTH | XCB_CONFIG_WINDOW_HEIGHT, values);
                                xcb_flush(context_.connection);
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
            	        	    xcb_flush(context_.connection);
            	        	}
            	        }
            	        break;
            	        case moving_mode::size_ns_top:
            	        {
            	            uint32_t width = ws.width();
            	            uint32_t height = ws.height() - y_mouse;
                            if (width > min_width && height > min_height)
            	            {
                                uint32_t values[] = { static_cast<uint32_t>(ws.left), static_cast<uint32_t>(ev->root_y), width, height };
                                xcb_configure_window(context_.connection, context_.wnd, XCB_CONFIG_WINDOW_X | XCB_CONFIG_WINDOW_Y |
                                    XCB_CONFIG_WINDOW_WIDTH | XCB_CONFIG_WINDOW_HEIGHT, values);
                                xcb_flush(context_.connection);
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
            	        	    xcb_flush(context_.connection);
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
                                xcb_flush(context_.connection);
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
            	        	    xcb_flush(context_.connection);
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
                                xcb_flush(context_.connection);
            	            }
            	        }
            	        break;
            	        case moving_mode::size_nesw_bottom:
            	        {
            	            uint32_t width = ws.width() - x_mouse;
            	            uint32_t height = y_mouse;
                            if (width > min_width && height > min_height)
            	            {
            	            	uint32_t values[] = { static_cast<uint32_t>(ev->root_x), static_cast<uint32_t>(ws.left), width, height };
                                xcb_configure_window(context_.connection, context_.wnd, XCB_CONFIG_WINDOW_X | XCB_CONFIG_WINDOW_Y |
                                    XCB_CONFIG_WINDOW_WIDTH | XCB_CONFIG_WINDOW_HEIGHT, values);
                                xcb_flush(context_.connection);
            	            }
            	        }
            	        break;
                    }

                    /*ws = get_window_size(context_);
                    update_position(ws);
                    update_buttons(false);
                    if (size_change_callback)
                    {
                        size_change_callback(ws.width(), ws.height());
                    }*/
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
                    x_click = ev->event_x;
                    y_click = ev->event_y;

                    auto ws = get_window_size(context_);

                    if (!send_mouse_event({ mouse_event_type::left_down, x_click, y_click }) &&
                        (flag_is_set(window_style_, window_style::moving) && window_state_ == window_state::normal))
                    {
                        moving_mode_ = moving_mode::move;

                        if (flag_is_set(window_style_, window_style::resizable) && window_state_ == window_state::normal)
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
            break;
            case XCB_BUTTON_RELEASE:
            {
                moving_mode_ = moving_mode::none;

                auto *ev = (xcb_button_press_event_t *)e;
                if (ev->detail == 1)
                {
                    send_mouse_event({ mouse_event_type::left_up, ev->event_x, ev->event_y });
                }
            }
            break;
            case XCB_LEAVE_NOTIFY:
            	send_mouse_event({ mouse_event_type::leave, -1, -1 });
            break;
            case XCB_CLIENT_MESSAGE:
            	if((*(xcb_client_message_event_t*)e).data.data32[0] == (*wm_delete_msg).atom)
                {
                    xcb_destroy_window(context_.connection, context_.wnd);
            	    xcb_disconnect(context_.connection);

                    context_.wnd = 0;
                    context_.connection = nullptr;

                    runned = false;

                    if (parent && close_callback)
                    {
                        close_callback();
                    }
                }
            break;
        }
	    free(e);
    }
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
    	event.data.data32[0] = (*wm_delete_msg).atom;

    	xcb_send_event(context_.connection, false, context_.wnd, XCB_EVENT_MASK_NO_EVENT, (const char*)&event);
    	xcb_flush(context_.connection);
    }
}

#endif

}
