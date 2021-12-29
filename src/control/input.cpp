//
// Copyright (c) 2021-2022 Anton Golovkov (udattsk at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/ud84/WUI
//

#include <wui/control/input.hpp>

#include <wui/window/window.hpp>

#include <wui/theme/theme.hpp>

namespace wui
{

input::input(const std::wstring &text__, input_view input_view__, std::shared_ptr<i_theme> theme__)
    : input_view_(input_view__),
    text_(text__),
    change_callback(),
    theme_(theme__),
    position_(),
    parent(),
    showed_(true), enabled_(true),
    focused_(false),
    focusing_(true)
#ifdef _WIN32
    , background_brush(0), selection_brush(0),
    cursor_pen(0), border_pen(0), focused_border_pen(0)
#endif
{
#ifdef _WIN32
    make_primitives();
#endif
}

input::~input()
{
#ifdef _WIN32
    destroy_primitives();
#endif

    if (parent.lock())
    {
        parent.lock()->remove_control(shared_from_this());
    }
}

void input::draw(graphic &gr)
{
    if (!showed_)
    {
        return;
    }

#ifdef _WIN32
    RECT text_rect = { 0 };
    DrawTextW(gr.dc, text_.c_str(), static_cast<int32_t>(text_.size()), &text_rect, DT_CALCRECT);

    SelectObject(gr.dc, !focused_ ? border_pen : focused_border_pen);
    SelectObject(gr.dc, background_brush);

    auto rnd = theme_dimension(theme_value::button_round, theme_);
    RoundRect(gr.dc, position_.left, position_.top, position_.right, position_.bottom, rnd, rnd);
	
    SetTextColor(gr.dc, theme_color(theme_value::input_text, theme_));
    SetBkColor(gr.dc, theme_color(theme_value::input_background, theme_));

    TextOutW(gr.dc, position_.left + 5, position_.top + 3, text_.c_str(), (int32_t)text_.size());
#endif
}

void input::receive_event(const event &ev)
{
    if (!showed_ || !enabled_)
    {
        return;
    }

    if (ev.type == event_type::mouse)
    {
        switch (ev.mouse_event_.type)
        {
            case mouse_event_type::left_down:
                
            break;
            case mouse_event_type::left_up:
                
            break;
        }
    }
    else if (ev.type == event_type::keyboard)
    {
        if (ev.keyboard_event_.type == keyboard_event_type::press)
        {
            text_ += ev.keyboard_event_.key;
            redraw();
        }
    }
}

void input::set_position(const rect &position__)
{
    auto prev_position = position_;
    position_ = position__;

    if (parent.lock())
    {
        parent.lock()->redraw(prev_position, true);
    }
	
    redraw();
}

rect input::position() const
{
    return position_;
}

void input::set_parent(std::shared_ptr<window> window_)
{
    parent = window_;
}

void input::clear_parent()
{
    parent.reset();
}

void input::set_focus()
{
    if (focusing_ && enabled_ && showed_)
    {
        focused_ = true;

        redraw();
    }
}

bool input::remove_focus()
{
    focused_ = false;

    redraw();

    return true;
}

bool input::focused() const
{
    return focused_;
}

bool input::focusing() const
{
    return enabled_ && showed_ && focusing_;
}

void input::update_theme(std::shared_ptr<i_theme> theme__)
{
    if (theme_ && !theme__)
    {
        return;
    }
    theme_ = theme__;

#ifdef _WIN32
    destroy_primitives();
    make_primitives();
#endif
}

void input::show()
{
    showed_ = true;
    redraw();
}

void input::hide()
{
    showed_ = false;
    if (parent.lock())
    {
        parent.lock()->redraw(position_, true);
    }
}

bool input::showed() const
{
    return showed_;
}

void input::enable()
{
    enabled_ = true;
    redraw();
}

void input::disable()
{
    enabled_ = false;
    redraw();
}

bool input::enabled() const
{
    return enabled_;
}

void input::set_text(const std::wstring &text__)
{
    text_ = text__;
    redraw();
}

std::wstring input::text() const
{
    return text_;
}

void input::set_input_view(input_view input_view__)
{
    input_view_ = input_view__;
}

void input::set_change_callback(std::function<void(const std::wstring&)> change_callback_)
{
    change_callback = change_callback_;
}

void input::redraw()
{
    if (parent.lock())
    {
        parent.lock()->redraw(position_);
    }
}

#ifdef _WIN32
void input::make_primitives()
{
    background_brush = CreateSolidBrush(theme_color(theme_value::input_background, theme_));
    selection_brush = CreateSolidBrush(theme_color(theme_value::input_selection, theme_));
    cursor_pen = CreatePen(PS_SOLID, 1, theme_color(theme_value::input_cursor, theme_));
    border_pen = CreatePen(PS_SOLID, 1, theme_color(theme_value::input_border, theme_));
    focused_border_pen = CreatePen(PS_SOLID, 1, theme_color(theme_value::input_focused_border, theme_));
}

void input::destroy_primitives()
{
    DeleteObject(background_brush);
    DeleteObject(selection_brush);
    DeleteObject(cursor_pen);
    DeleteObject(border_pen);
    DeleteObject(focused_border_pen);
}
#endif

}
