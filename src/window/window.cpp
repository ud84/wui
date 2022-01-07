//
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

#include <algorithm>

#ifdef _WIN32
#include <windowsx.h>
#include <resource.hpp>

#elif __linux__

#include <X11/Xatom.h>
#include <X11/Xcursor/Xcursor.h>

#include <wui/common/char_helpers.hpp>

#endif

namespace wui
{

window::window()
    : controls(),
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
    hwnd(0),
    background_brush(0),
    font(0),
    mouse_tracked(false)
#elif __linux__
    pin_button(new button(L"", std::bind(&window::pin, this), button_view::only_image, L"", 24)),
    minimize_button(new button(L"", std::bind(&window::minimize, this), button_view::only_image, L"", 24)),
    expand_button(new button(L"", [this]() { window_state_ == window_state::normal ? expand() : normal(); }, button_view::only_image, window_state_ == window_state::normal ? L"" : L"", 24)),
    close_button(new button(L"", std::bind(&window::destroy, this), button_view::only_image, L"", 24)),
	display(nullptr),
	wnd(0),
	xft_font(nullptr),
	xft_draw(nullptr),
	title_color(),
	wm_delete_message(0),
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
        InvalidateRect(hwnd, &invalidatingRect, clear ? TRUE : FALSE);
#endif
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
    SetWindowPos(hwnd, NULL, position_.left, position_.top, position_.right, position_.bottom, NULL);
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
        DestroyWindow(hwnd);
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

    destroy_primitives();
    make_primitives();

#ifdef _WIN32

    if (!parent)
    {
        RECT client_rect;
        GetClientRect(hwnd, &client_rect);
        InvalidateRect(hwnd, &client_rect, TRUE);
    }
#elif __linux__
    if (!parent)
    {
        XSetWindowBackground(display, wnd, theme_color(theme_value::window_background, theme_));
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
        ShowWindow(hwnd, SW_SHOW);
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
        ShowWindow(hwnd, SW_HIDE);
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
    ShowWindow(hwnd, SW_MINIMIZE);
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

        SetWindowPos(hwnd, NULL, work_area.left, work_area.top, work_area.right, work_area.bottom, NULL);
    }
    else
    {
        ShowWindow(hwnd, SW_MAXIMIZE);
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
    EnableWindow(hwnd, FALSE);
#endif
}

void window::unlock()
{
#ifdef _WIN32
    EnableWindow(hwnd, TRUE);
    SetWindowPos(hwnd, HWND_TOP, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE);
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
    make_primitives();

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

    hwnd = CreateWindowW(wcex.lpszClassName, L"", WS_VISIBLE | WS_POPUP,
        position_.left, position_.top, position_.right, position_.bottom, nullptr, nullptr, h_inst, this);

    if (!hwnd)
    {
        return false;
    }

    SetWindowText(hwnd, caption.c_str());

    UpdateWindow(hwnd);

#elif __linux__

    display = XOpenDisplay(nullptr);
    if (!display)
    {
    	fprintf(stderr, "window can't open default display\n");
    	return false;
    }

    wm_delete_message = XInternAtom(display, "WM_DELETE_WINDOW", False);

    auto screen_number = DefaultScreen(display);

    wnd = XCreateSimpleWindow(display,
        RootWindow(display, screen_number),
        position_.left, position_.right, position_.width(), position_.height(), 0,
		theme_color(theme_value::window_background, theme_),
		theme_color(theme_value::window_background, theme_));

    make_primitives();

    XSetWMProtocols(display, wnd, &wm_delete_message, 1);

    /// Fullscreen
    /*auto window_type = XInternAtom(display, "_NET_WM_STATE", False);
    auto value = XInternAtom(display, "_NET_WM_STATE_FULLSCREEN", False);
    XChangeProperty(display, wnd, window_type, XA_ATOM, 32, PropModeReplace, reinterpret_cast<unsigned char*>(&value), 1);*/

    auto window_type = XInternAtom(display, "_NET_WM_WINDOW_TYPE", False);
    auto value = XInternAtom(display, "_NET_WM_WINDOW_TYPE_TOOLBAR", False);
    XChangeProperty(display, wnd, window_type, XA_ATOM, 32, PropModeReplace, reinterpret_cast<unsigned char*>(&value), 1);

    XSelectInput(display, wnd,
        ExposureMask |
        KeyPressMask |
        PointerMotionMask |
        ButtonPressMask |
        ButtonReleaseMask |
        LeaveWindowMask);

    XMapWindow(display, wnd);

    runned = true;
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
        DestroyWindow(hwnd);
#elif __linux__
        send_destroy_event();
#endif
    }
}

void window::make_primitives()
{
#ifdef _WIN32
    background_brush = CreateSolidBrush(theme_color(theme_value::window_background, theme_));
    font = CreateFont(theme_dimension(theme_value::window_title_font_size, theme_), 0, 0, 0, FW_DONTCARE, FALSE, FALSE, FALSE, ANSI_CHARSET,
        OUT_TT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY,
        DEFAULT_PITCH | FF_DONTCARE, theme_string(theme_value::window_title_font_name, theme_).c_str());

#elif __linux__

    if (!xft_font)
    {
    	auto scr = DefaultScreen(display);
    	auto visual = DefaultVisual(display, scr);
    	auto cmap = DefaultColormap(display, scr);

    	std::string font_name = to_multibyte(theme_string(theme_value::window_title_font_name, theme_));
        std::string font_size =  std::to_string(theme_dimension(theme_value::window_title_font_size, theme_));
        std::string font_query = font_name + ":size=" + font_size + ":antialias=true";

        xft_draw = XftDrawCreate(display, wnd, visual, cmap);
        if (!xft_draw)
        {
        	fprintf(stderr, "window XftDrawCreate error\n");
        	return;
        }

        auto txt_color = theme_color(theme_value::window_text, theme_);
        XRenderColor xr_color = { static_cast<unsigned short>(0xffff * get_red(txt_color) / 0xff),
            static_cast<unsigned short>(0xffff * get_green(txt_color) / 0xff),
            static_cast<unsigned short>(0xffff * get_blue(txt_color) / 0xff),
            0xffff };
        if (!XftColorAllocValue(display, visual, cmap, &xr_color, &title_color))
        {
            fprintf(stderr, "cannot allocate xft color for window title\n");
            return;
        }

        xft_font = XftFontOpenName(display, scr, font_query.c_str());
        if (!xft_font)
        {
            fprintf(stderr, "window can't load the font %s\n", font_name.c_str());
        }
    }
#endif
}

void window::destroy_primitives()
{
#ifdef _WIN32
    DeleteObject(background_brush);
    DeleteObject(font);
#elif __linux__
    if (xft_draw)
    {
    	auto scr = DefaultScreen(display);
    	XftColorFree(display, DefaultVisual(display, scr), DefaultColormap(display, scr), &title_color);
        XftDrawDestroy(xft_draw);
        xft_draw = nullptr;
    }
#endif
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
            HDC hdc = BeginPaint(hwnd, &ps);

            HDC mem_dc = CreateCompatibleDC(hdc);

            RECT client_rect;
            GetClientRect(hwnd, &client_rect);
            
            HBITMAP mem_bitmap = CreateCompatibleBitmap(hdc, client_rect.right, client_rect.bottom);
            SelectObject(mem_dc, mem_bitmap);

            FillRect(mem_dc, &client_rect, wnd->background_brush);

            SelectObject(mem_dc, wnd->font);

            SetTextColor(mem_dc, theme_color(theme_value::window_text, wnd->theme_));
            SetBkMode(mem_dc, TRANSPARENT);

            if (flag_is_set(wnd->window_style_, window_style::title_showed))
            {
                TextOutW(mem_dc, 5, 5, wnd->caption.c_str(), (int32_t)wnd->caption.size());
            }
		
            graphic gr{ mem_dc };
            for (auto &control : wnd->controls)
            {
                control->draw(gr);
            }

            BitBlt(hdc,
                0,
                0,
                client_rect.right,
                client_rect.bottom,
                mem_dc,
                0,
                0,
                SRCCOPY);

            DeleteObject(mem_bitmap);
            DeleteDC(mem_dc);

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

            if (flag_is_set(wnd->window_style_, window_style::resizable) && wnd->window_state_ == window_state::normal)
            {
                if ((x_mouse > window_rect.right - window_rect.left - 5 && y_mouse > window_rect.bottom - window_rect.top - 5) ||
                    (x_mouse < 5 && y_mouse < 5))
                {
                    SetCursor(LoadCursor(NULL, IDC_SIZENWSE));
                }
                else if ((x_mouse > window_rect.right - window_rect.left - 5 && y_mouse < 5) ||
                    (x_mouse < 5 && y_mouse > window_rect.bottom - window_rect.top - 5))
                {
                    SetCursor(LoadCursor(NULL, IDC_SIZENESW));
                }
                else if (x_mouse > window_rect.right - window_rect.left - 5 || x_mouse < 5)
                {
                    SetCursor(LoadCursor(NULL, IDC_SIZEWE));
                }
                else if (y_mouse > window_rect.bottom - window_rect.top - 5 || y_mouse < 5)
                {
                    SetCursor(LoadCursor(NULL, IDC_SIZENS));
                }
                else if (!wnd->active_control)
                {
                    SetCursor(LoadCursor(NULL, IDC_ARROW));
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
            wnd->destroy_primitives();
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

void window::send_destroy_event()
{
    if (display)
    {
        XEvent ev = { 0 };
        ev.xclient.type = ClientMessage;
        ev.xclient.window = wnd;
        ev.xclient.message_type = XInternAtom(display, "WM_PROTOCOLS", true);
        ev.xclient.format = 32;
        ev.xclient.data.l[0] = wm_delete_message;

        XSendEvent(display, wnd, True, NoEventMask, &ev);
    }
}

rect window::get_mouse_screen_position()
{
    Window window_returned;
    int root_x, root_y;
    int win_x, win_y;
    unsigned int mask_return;
    XQueryPointer(display, XRootWindow(display, 0), &window_returned,
        &window_returned, &root_x, &root_y, &win_x, &win_y, &mask_return);

    return rect{ root_x, root_y, root_x, root_y };
}

void window::process_events()
{
    XEvent e;

    while(runned)
    {
        XNextEvent(display, &e);

        switch (e.type)
        {
            case Expose:
            {
                if (e.xexpose.count != 0)
                {
                    break;
                }

                auto gc = XCreateGC(display, wnd, 0, NULL);

                if (flag_is_set(window_style_, window_style::title_showed))
                {
                    if (xft_font)
                    {
                    	XftDrawString8(xft_draw, &title_color, xft_font, 5, 15, (const FcChar8 *)to_multibyte(caption).c_str(), caption.size());
                    }
                }

                graphic gr{ gc };
                for (auto &control : controls)
                {
                    control->draw(gr);
                }

                XFreeGC(display, gc);
                XFlush(display);
            }
            break;
            case KeyPress:
                destroy();
            break;
            case MotionNotify:
            {
                XWindowAttributes wnd_attr;
                XGetWindowAttributes(display, wnd, &wnd_attr);

                int16_t x_mouse = e.xbutton.x;
                int16_t y_mouse = e.xbutton.y;

                if (flag_is_set(window_style_, window_style::resizable) && window_state_ == window_state::normal)
                {
                    if ((x_mouse > wnd_attr.width - 5 && y_mouse > wnd_attr.height - 5) ||
                        (x_mouse < 5 && y_mouse < 5))
                    {
                    	XDefineCursor(display, wnd, XcursorLibraryLoadCursor(display, "top_left_corner"));
                    }
                    else if ((x_mouse > wnd_attr.width - 5 && y_mouse < 5) ||
                        (x_mouse < 5 && y_mouse > wnd_attr.height - 5))
                    {
                    	XDefineCursor(display, wnd, XcursorLibraryLoadCursor(display, "top_right_corner"));
                    }
                    else if (x_mouse > wnd_attr.width - 5 || x_mouse < 5)
                    {
                    	XDefineCursor(display, wnd, XcursorLibraryLoadCursor(display, "sb_h_double_arrow"));
                    }
                    else if (y_mouse > wnd_attr.height - 5 || y_mouse < 5)
                    {
                    	XDefineCursor(display, wnd, XcursorLibraryLoadCursor(display, "sb_v_double_arrow"));
                    }
                    else if (!active_control)
                    {
                    	XDefineCursor(display, wnd, XcursorLibraryLoadCursor(display, "default"));
                    }
                }

                if (moving_mode_ != moving_mode::none)
                {
                    switch (moving_mode_)
                    {
                        case moving_mode::move:
                        {
                            int32_t x_window = wnd_attr.x + x_mouse - x_click;
                            int32_t y_window = wnd_attr.y + y_mouse - y_click;

                            XMoveWindow(display, wnd, x_window, y_window);
                        }
                        break;
                        case moving_mode::size_we_left:
                        {
                            int32_t width = wnd_attr.width - x_mouse;
                            int32_t height = wnd_attr.height;
                            if (width > min_width && height > min_height)
                            {
                                XMoveResizeWindow(display, wnd, get_mouse_screen_position().left, wnd_attr.y, width, height);
                            }
                        }
            	        break;
            	        case moving_mode::size_we_right:
            	        {
                            int32_t width = x_mouse;
                            int32_t height = wnd_attr.height;
                            if (width > min_width && height > min_height)
                            {
            	        	    XResizeWindow(display, wnd, width, height);
            	        	}
            	        }
            	        break;
            	        case moving_mode::size_ns_top:
            	        {
            	            int32_t width = wnd_attr.width;
            	            int32_t height = wnd_attr.height - y_mouse;
                            if (width > min_width && height > min_height)
            	            {
            	                XMoveResizeWindow(display, wnd, wnd_attr.x, get_mouse_screen_position().top, width, height);
            	            }
            	        }
            	        break;
            	        case moving_mode::size_ns_bottom:
            	        {
            	            int32_t width = wnd_attr.width;
            	            int32_t height = y_mouse;
                            if (width > min_width && height > min_height)
            	            {
            	                XResizeWindow(display, wnd, width, height);
            	            }
            	        }
            	        break;
            	        case moving_mode::size_nesw_top:
            	        {
            	            int32_t width = x_mouse;
            	            int32_t height = wnd_attr.height - y_mouse;
                            if (width > min_width && height > min_height)
            	            {
            	            	XMoveResizeWindow(display, wnd, wnd_attr.x, get_mouse_screen_position().top, width, height);
            	            }
            	        }
            	        break;
            	        case moving_mode::size_nwse_bottom:
            	        {
            	            int32_t width = x_mouse;
            	            int32_t height = y_mouse;
                            if (width > min_width && height > min_height)
            	            {
            	                XResizeWindow(display, wnd, width, height);
            	            }
            	        }
            	        break;
            	        case moving_mode::size_nwse_top:
            	        {
            	            auto mouse_pos = get_mouse_screen_position();
            	            int32_t width = wnd_attr.width - x_mouse;
            	            int32_t height = wnd_attr.height - y_mouse;
                            if (width > min_width && height > min_height)
            	            {
            	                XMoveResizeWindow(display, wnd, mouse_pos.left, mouse_pos.top, width, height);
            	            }
            	        }
            	        break;
            	        case moving_mode::size_nesw_bottom:
            	        {
            	            int32_t width = wnd_attr.width - x_mouse;
            	            int32_t height = y_mouse;
                            if (width > min_width && height > min_height)
            	            {
            	                XMoveResizeWindow(display, wnd, get_mouse_screen_position().left, wnd_attr.y, width, height);
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
            case ButtonPress:
                if (e.xbutton.button == Button1)
                {
                	XWindowAttributes wnd_attr;
                	XGetWindowAttributes(display, wnd, &wnd_attr);

                    x_click = e.xbutton.x;
                    y_click = e.xbutton.y;

                    if (!send_mouse_event({ mouse_event_type::left_down, x_click, y_click }) &&
                        (flag_is_set(window_style_, window_style::moving) && window_state_ == window_state::normal))
                    {
                        moving_mode_ = moving_mode::move;

                        if (flag_is_set(window_style_, window_style::resizable) && window_state_ == window_state::normal)
                        {
                            if (x_click > wnd_attr.width - 5 && y_click > wnd_attr.height - 5)
                            {
                                moving_mode_ = moving_mode::size_nwse_bottom;
                            }
                            else if (x_click < 5 && y_click < 5)
                            {
                                moving_mode_ = moving_mode::size_nwse_top;
                            }
                            else if (x_click > wnd_attr.width - 5 && y_click < 5)
                            {
                                moving_mode_ = moving_mode::size_nesw_top;
                            }
                            else if (x_click < 5 && y_click > wnd_attr.height - 5)
                            {
                                moving_mode_ = moving_mode::size_nesw_bottom;
                            }
                            else if (x_click > wnd_attr.width - 5)
                            {
                                moving_mode_ = moving_mode::size_we_right;
                            }
                            else if (x_click < 5)
                            {
                                moving_mode_ = moving_mode::size_we_left;
                            }
                            else if (y_click > wnd_attr.height - 5)
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
            break;
            case ButtonRelease:
                moving_mode_ = moving_mode::none;

                send_mouse_event({ mouse_event_type::left_up, e.xbutton.x, e.xbutton.y });
            break;
            case LeaveNotify:
            	send_mouse_event({ mouse_event_type::leave, -1, -1 });
            break;
            case ClientMessage:
                if (e.xclient.data.l[0] == wm_delete_message)
                {
                	destroy_primitives();
                    XDestroyWindow(display, e.xclient.window);
                    XCloseDisplay(display);
                    display = nullptr;

                    runned = false;

                    if (parent && close_callback)
                    {
                        close_callback();
                    }
                }
            break;
        }
    }
}

#endif

}
