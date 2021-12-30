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
    cursor_position(0),
    parent(),
    timer_(std::bind(&input::redraw_cursor, this)),
    showed_(true), enabled_(true),
    focused_(false),
    focusing_(true),
    cursor_visible(false)
#ifdef _WIN32
    , background_brush(0), selection_brush(0),
    cursor_pen(0), background_pen(0), border_pen(0), focused_border_pen(0)
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

    auto parent_ = parent.lock();
    if (parent_)
    {
        parent_->remove_control(shared_from_this());
    }
}

#ifdef _WIN32

rect calculate_text_dimensions(HDC dc, std::wstring text, size_t text_length)
{
    RECT text_rect = { 0 };

    text.resize(text_length);

    if (text_length == 0)
    {
        text = L"A";
    }

    DrawTextW(dc, text.c_str(), static_cast<int32_t>(text.size()), &text_rect, DT_CALCRECT);

    return { 0, 0, text_length != 0 ? text_rect.right : 0, text_rect.bottom };
}

#endif

rect input::calculate_cursor_coordinates(int32_t text_width, int32_t text_height)
{
    return rect{ position_.left + left_indent + text_width, position_.top + top_indent, position_.left + left_indent + text_width, position_.top + top_indent + text_height };
}

void input::draw(graphic &gr)
{
    if (!showed_)
    {
        return;
    }

#ifdef _WIN32
    SelectObject(gr.dc, !focused_ ? border_pen : focused_border_pen);
    SelectObject(gr.dc, background_brush);

    auto rnd = theme_dimension(theme_value::button_round, theme_);
    RoundRect(gr.dc, position_.left, position_.top, position_.right, position_.bottom, rnd, rnd);
	
    SetTextColor(gr.dc, theme_color(theme_value::input_text, theme_));
    SetBkColor(gr.dc, theme_color(theme_value::input_background, theme_));

    TextOutW(gr.dc, position_.left + left_indent, position_.top + top_indent, text_.c_str(), (int32_t)text_.size());

    SelectObject(gr.dc, cursor_visible ? cursor_pen : background_pen);

    auto text_dimensions = calculate_text_dimensions(gr.dc, text_, cursor_position);
    auto cursor_coordinates = calculate_cursor_coordinates(text_dimensions.right, text_dimensions.bottom);

    MoveToEx(gr.dc, cursor_coordinates.left, cursor_coordinates.top, (LPPOINT)NULL);
    LineTo(gr.dc, cursor_coordinates.right, cursor_coordinates.bottom);
    
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
        switch (ev.keyboard_event_.type)
        {
            case keyboard_event_type::down:
                timer_.stop();
                cursor_visible = true;
                switch (ev.keyboard_event_.key)
                {
                    case vk_left:
                        if (cursor_position > 0)
                        {
                            --cursor_position;
                            redraw();
                        }
                    break;
                    case vk_right:
                        if (cursor_position < text_.size())
                        {
                            ++cursor_position;
                            redraw();
                        }
                    break;
                    case vk_home:
                        cursor_position = 0;
                        redraw();
                    break;
                    case vk_end:
                        if (!text_.empty())
                        {
                            cursor_position = text_.size();
                            redraw();
                        }
                    break;
                    case vk_back:
                        if (cursor_position > 0)
                        {
                            text_.erase(cursor_position - 1, 1);
                            --cursor_position;
                            redraw();
                        }
                    break;
                    case vk_del:
                        if (!text_.empty())
                        {
                            text_.erase(cursor_position, 1);
                            redraw();
                        }
                    break;
                }
            break;
            case keyboard_event_type::up:
                timer_.start();
            break;
            case keyboard_event_type::key:
                if (input_view_ == input_view::singleline && ev.keyboard_event_.key == vk_return)
                {
                    return;
                }
                text_.insert(cursor_position, 1, ev.keyboard_event_.key);
                ++cursor_position;
                redraw();
            break;
        }
    }
}

void input::set_position(const rect &position__)
{
    auto prev_position = position_;
    position_ = position__;

    auto parent_ = parent.lock();
    if (parent_)
    {
        parent_->redraw(prev_position, true);
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

        timer_.start(500);
    }
}

bool input::remove_focus()
{
    focused_ = false;

    cursor_visible = false;

    timer_.stop();

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
    auto parent_ = parent.lock();
    if (parent_)
    {
        parent_->redraw(position_, true);
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
    auto parent_ = parent.lock();
    if (parent_)
    {
        parent_->redraw(position_);
    }
}

void input::redraw_cursor()
{
    cursor_visible = !cursor_visible;
    redraw();
}

#ifdef _WIN32
void input::make_primitives()
{
    background_brush = CreateSolidBrush(theme_color(theme_value::input_background, theme_));
    selection_brush = CreateSolidBrush(theme_color(theme_value::input_selection, theme_));
    cursor_pen = CreatePen(PS_SOLID, 1, theme_color(theme_value::input_cursor, theme_));
    background_pen = CreatePen(PS_SOLID, 1, theme_color(theme_value::input_background, theme_));
    border_pen = CreatePen(PS_SOLID, 1, theme_color(theme_value::input_border, theme_));
    focused_border_pen = CreatePen(PS_SOLID, 1, theme_color(theme_value::input_focused_border, theme_));
}

void input::destroy_primitives()
{
    DeleteObject(background_brush);
    DeleteObject(selection_brush);
    DeleteObject(cursor_pen);
    DeleteObject(background_pen);
    DeleteObject(border_pen);
    DeleteObject(focused_border_pen);
}
#endif

}
