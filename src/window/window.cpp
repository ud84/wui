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

#include <algorithm>

#ifdef _WIN32
#include <windowsx.h>
#include <resource.hpp>
#endif

namespace wui
{

window::window()
    : controls(),
    active_control(),
    window_type_(window_type::frame),
    position_(), normal_position(),
    caption(),
    window_state_(window_state::normal),
    theme_(),
    showed_(true), enabled_(true), title_showed(true),
    focused_index(0),
    parent(),
    moving_mode_(moving_mode::move),
    close_callback(),
    size_change_callback(),
    buttons_theme(make_custom_theme()), close_button_theme(make_custom_theme()),
#ifdef _WIN32
    minimize_button(new button(L"", std::bind(&window::minimize, this), button_view::only_image, IDB_WINDOW_MINIMIZE, 24)),
    expand_button(new button(L"", [this]() { window_state_ == window_state::normal ? expand() : normal(); }, button_view::only_image, window_state_ == window_state::normal ? IDB_WINDOW_EXPAND : IDB_WINDOW_NORMAL, 24)),
    close_button(new button(L"", std::bind(&window::destroy, this), button_view::only_image, IDB_WINDOW_CLOSE, 24)),
    hwnd(0),
    background_brush(0),
    font(0),
    x_click(0), y_click(0),
    mouse_tracked(false)
#elif __linux__
    minimize_button(new button(L"", std::bind(&window::minimize, this), button_view::only_image, ImagesConsts::Window_MinimizeButton, 24)),
    expand_button(new button(L"", [this]() { window_state_ == window_state::normal ? expand() : normal(); }, button_view::only_image, window_state_ == window_state::normal ? ImagesConsts::Window_ExpandButton : ImagesConsts::Window_NormalButton, 24)),
    close_button(new button(L"", std::bind(&window::destroy, this), button_view::only_image, ImagesConsts::Window_CloseButton, 24))
#endif
{
    minimize_button->disable_focusing();
    expand_button->disable_focusing();
    close_button->disable_focusing();

#ifdef _WIN32
    make_primitives();
#endif
}

window::~window()
{
    if (parent)
    {
        parent->remove_control(shared_from_this());
    }
#ifdef _WIN32
    destroy_primitives();
#endif
}

void window::add_control(std::shared_ptr<i_control> control, const rect &control_position)
{
    if (std::find(controls.begin(), controls.end(), control) == controls.end())
    {
        control->set_position(!parent ? control_position : position_ + control_position);
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
}

void window::clear_parent()
{
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
    destroy_primitives();
    make_primitives();

    if (!parent)
    {
        RECT client_rect;
        GetClientRect(hwnd, &client_rect);
        InvalidateRect(hwnd, &client_rect, TRUE);
    }
#endif

    for (auto &control : controls)
    {
        control->update_theme(theme_);
    }

    update_control_buttons_theme();
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
    if (title_showed)
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

window_state window::window_state() const
{
    return window_state_;
}

void window::show_title()
{
    title_showed = true;

    minimize_button->show();
    expand_button->show();
    close_button->show();

    redraw({ 0, 0, position_.width(), 30 }, false);
}

void window::hide_title()
{
    title_showed = false;

    minimize_button->hide();
    expand_button->hide();
    close_button->hide();

    redraw({ 0, 0, position_.width(), 30 }, true);
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

bool window::send_mouse_event(const mouse_event &ev)
{
    if (active_control && !active_control->position().in(ev.x, ev.y))
    {
        mouse_event me{ mouse_event_type::leave, 0, 0 };
        active_control->receive_event({ event_type::mouse, me });

        active_control.reset();
    }

    for (auto &control : controls)
    {
        if (control->position().in(ev.x, ev.y))
        {
            if (active_control == control)
            {
                if (ev.type == mouse_event_type::left_up)
                {
                    set_focused(control);
                }

                control->receive_event({ event_type::mouse, ev });
            }
            else
            {
                if (active_control)
                {
                    mouse_event me{ mouse_event_type::leave, 0, 0 };
                    active_control->receive_event({ event_type::mouse, me });
                }

                active_control = control;

                mouse_event me{ mouse_event_type::enter, 0, 0 };
                control->receive_event({ event_type::mouse, me });
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

void window::update_control_buttons_theme()
{
    auto background_color = theme_color(theme_value::window_background, theme_);

    if (window_type_ == window_type::frame)
    {
        buttons_theme->set_color(theme_value::button_calm, background_color);
        buttons_theme->set_color(theme_value::button_active, theme_color(theme_value::window_active_button, theme_));
        buttons_theme->set_color(theme_value::button_border, background_color);
        buttons_theme->set_color(theme_value::button_text, theme_color(theme_value::window_text, theme_));
        buttons_theme->set_color(theme_value::button_disabled, background_color);
        buttons_theme->set_dimension(theme_value::button_round, 0);
        buttons_theme->set_string(theme_value::images_path, theme_string(theme_value::images_path, theme_));

        minimize_button->update_theme(buttons_theme);
        expand_button->update_theme(buttons_theme);
    }

    close_button_theme->set_color(theme_value::button_calm, background_color);
    close_button_theme->set_color(theme_value::button_active, make_color(235, 15, 20));
    close_button_theme->set_color(theme_value::button_border, background_color);
    close_button_theme->set_color(theme_value::button_text, theme_color(theme_value::window_text, theme_));
    close_button_theme->set_color(theme_value::button_disabled, background_color);
    close_button_theme->set_dimension(theme_value::button_round, 0);
    close_button_theme->set_string(theme_value::images_path, theme_string(theme_value::images_path, theme_));

    close_button->update_theme(close_button_theme);
}

bool window::init(window_type type, const rect &position__, const std::wstring &caption_, std::function<void(void)> close_callback_, std::shared_ptr<i_theme> theme__)
{
    window_type_ = type;
    position_ = position__;
    normal_position = position_;
    caption = caption_;
    close_callback = close_callback_;
    theme_ = theme__;

    if (parent)
    {
        showed_ = true;
        parent->redraw(position_);

        return true;
    }

    update_control_buttons_theme();

    if (type == window_type::frame)
    {
        add_control(minimize_button, { position_.right - 78, 0, position_.right - 52, 26 });
        add_control(expand_button, { position_.right - 52, 0, position_.right - 26, 26 });
    }
    add_control(close_button, { position_.right - 26, 0, position_.right, 26 });

#ifdef _WIN32
    auto h_inst = GetModuleHandle(NULL);

    WNDCLASSEXW wcex = { 0 };

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style = CS_DBLCLKS;
    wcex.lpfnWndProc = window::wnd_proc;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = sizeof(this);
    wcex.hInstance = h_inst;
    wcex.hbrBackground = background_brush;
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
    }
    else
    {
#ifdef _WIN32
        DestroyWindow(hwnd);
#endif
    }

    if (close_callback)
    {
        close_callback();
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

            graphic gr{ hdc };

            SelectObject(hdc, wnd->font);

            SetTextColor(hdc, theme_color(theme_value::window_text, wnd->theme_));
            SetBkMode(hdc, TRANSPARENT);

            if (wnd->title_showed)
            {
                TextOutW(hdc, 5, 5, wnd->caption.c_str(), (int32_t)wnd->caption.size());
            }
		
            for (auto &control : wnd->controls)
            {
                control->draw(gr);
            }

            EndPaint(hwnd, &ps);
        }
        break;
        case WM_ERASEBKGND:
        {
            HDC hdc = (HDC)w_param;
            RECT client_rect;
            GetClientRect(hwnd, &client_rect);
            SetMapMode(hdc, MM_ANISOTROPIC);

            window* wnd = reinterpret_cast<window*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));
            FillRect(hdc, &client_rect, wnd->background_brush);
        }
        break;
        case WM_MOUSEMOVE:
        {
            window* wnd = reinterpret_cast<window*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));

            RECT window_rect;
            GetWindowRect(hwnd, &window_rect);

            int16_t x_mouse = GET_X_LPARAM(l_param);
            int16_t y_mouse = GET_Y_LPARAM(l_param);

            if (wnd->window_type_ == window_type::frame && wnd->window_state_ == window_state::normal)
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

            if (GetCapture() == hwnd && wnd->window_state_ == window_state::normal)
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
                        SetWindowPos(hwnd, NULL, scr_mouse.x, window_rect.top, width, height, SWP_NOZORDER);
                    }
                    break;
                    case moving_mode::size_we_right:
                    {
                        int32_t width = x_mouse;
                        int32_t height = window_rect.bottom - window_rect.top;
                        SetWindowPos(hwnd, NULL, 0, 0, width, height, SWP_NOMOVE | SWP_NOZORDER);
                    }
                    break;
                    case moving_mode::size_ns_top:
                    {
                        POINT scr_mouse = { 0 };
                        GetCursorPos(&scr_mouse);

                        int32_t width = window_rect.right - window_rect.left;
                        int32_t height = window_rect.bottom - window_rect.top - y_mouse;
                        SetWindowPos(hwnd, NULL, window_rect.left, scr_mouse.y, width, height, SWP_NOZORDER);
                    }
                    break;
                    case moving_mode::size_ns_bottom:
                    {
                        int32_t width = window_rect.right - window_rect.left;
                        int32_t height = y_mouse;
                        SetWindowPos(hwnd, NULL, 0, 0, width, height, SWP_NOMOVE | SWP_NOZORDER);
                    }
                    break;
                    case moving_mode::size_nesw_top:
                    {
                        POINT scr_mouse = { 0 };
                        GetCursorPos(&scr_mouse);

                        int32_t width = x_mouse;
                        int32_t height = window_rect.bottom - window_rect.top - y_mouse;
                        SetWindowPos(hwnd, NULL, window_rect.left, scr_mouse.y, width, height, SWP_NOZORDER);
                    }
                    break;
                    case moving_mode::size_nwse_bottom:
                    {
                        int32_t width = x_mouse;
                        int32_t height = y_mouse;
                        SetWindowPos(hwnd, NULL, 0, 0, width, height, SWP_NOMOVE | SWP_NOZORDER);
                    }
                    break;
                    case moving_mode::size_nwse_top:
                    {
                        POINT scrMouse = { 0 };
                        GetCursorPos(&scrMouse);

                        int32_t width = window_rect.right - window_rect.left - x_mouse;
                        int32_t height = window_rect.bottom - window_rect.top - y_mouse;
                        SetWindowPos(hwnd, NULL, scrMouse.x, scrMouse.y, width, height, SWP_NOZORDER);
                    }
                    break;
                    case moving_mode::size_nesw_bottom:
                    {
                        POINT scr_mouse = { 0 };
                        GetCursorPos(&scr_mouse);

                        int32_t width = window_rect.right - window_rect.left - x_mouse;
                        int32_t height = y_mouse;
                        SetWindowPos(hwnd, NULL, scr_mouse.x, window_rect.top, width, height, SWP_NOZORDER);
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

            if (!wnd->send_mouse_event({ mouse_event_type::left_down, wnd->x_click, wnd->y_click }) && wnd->window_type_ == window_type::frame && wnd->window_state_ == window_state::normal)
            {
                wnd->moving_mode_ = moving_mode::move;

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
        break;
        case WM_LBUTTONUP:
        {
            ReleaseCapture();

            window* wnd = reinterpret_cast<window*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));

            wnd->moving_mode_ = moving_mode::none;

            wnd->send_mouse_event({ mouse_event_type::left_up, GET_X_LPARAM(l_param), GET_Y_LPARAM(l_param) });
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

            if (wnd->window_type_ == window_type::frame)
            {
                wnd->minimize_button->set_position({ width - 78, 0, width - 52, 26 });
                wnd->expand_button->set_position({ width - 52, 0, width - 26, 26 });
            }
            wnd->close_button->set_position({ width - 26, 0, width, 26 });

            wnd->update_position();
			
            if (wnd->size_change_callback)
            {
                wnd->size_change_callback(LOWORD(l_param), HIWORD(l_param));
            }
        }
        break;
        case WM_MOVE:
        {
            window* wnd = reinterpret_cast<window*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));
            wnd->update_position();
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
            if (wnd->close_callback)
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

void window::make_primitives()
{
    background_brush = CreateSolidBrush(theme_color(theme_value::window_background, theme_));
    font = CreateFont(theme_dimension(theme_value::window_title_font_size, theme_), 0, 0, 0, FW_DONTCARE, FALSE, FALSE, FALSE, ANSI_CHARSET,
        OUT_TT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY,
        DEFAULT_PITCH | FF_DONTCARE, theme_string(theme_value::window_title_font_name, theme_).c_str());
}

void window::destroy_primitives()
{
    DeleteObject(background_brush);
    DeleteObject(font);
}

void window::update_position()
{
    RECT window_rect = { 0 };
    GetWindowRect(hwnd, &window_rect);
    if (window_rect.left > 0 && window_rect.top > 0 && window_rect.left != window_rect.right && window_rect.top != window_rect.bottom)
    {
        position_ = { window_rect.left, window_rect.top, window_rect.right - window_rect.left, window_rect.bottom - window_rect.top };
        if (window_state_ != window_state::maximized)
        {
            normal_position = position_;
        }
    }
}

#endif

}
