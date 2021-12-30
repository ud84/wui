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

static const int32_t input_horizontal_indent = 5, input_top_indent = 3;

input::input(const std::wstring &text__, input_view input_view__, std::shared_ptr<i_theme> theme__)
    : input_view_(input_view__),
    text_(text__),
    change_callback(),
    theme_(theme__),
    position_(),
    cursor_position(0), select_start_position(0), select_end_position(0),
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

rect calculate_cursor_coordinates(int32_t text_width, int32_t text_height, const rect &position_)
{
    return rect{ position_.left + input_horizontal_indent + text_width,
        position_.top + input_top_indent,
        position_.left + input_horizontal_indent + text_width,
        position_.top + input_top_indent + text_height };
}

#ifdef _WIN32

rect calculate_text_dimensions(HDC dc, std::wstring text, size_t text_length)
{
    RECT text_rect = { 0 };

    text.resize(text_length);

    if (text_length == 0)
    {
        text = L"W";
    }

    DrawTextW(dc, text.c_str(), static_cast<int32_t>(text.size()), &text_rect, DT_CALCRECT);

    return { 0, 0, text_length != 0 ? text_rect.right : 0, text_rect.bottom };
}

void draw_text(HDC dc, const std::wstring &text, const rect &position_, std::shared_ptr<i_theme> &theme_)
{
    SetTextColor(dc, theme_color(theme_value::input_text, theme_));
    SetBkColor(dc, theme_color(theme_value::input_background, theme_));

    auto text_width = calculate_text_dimensions(dc, text, text.size()).right;

    if (text_width > position_.width() - (input_horizontal_indent * 2))
    {

    }

    TextOutW(dc, position_.left + input_horizontal_indent, position_.top + input_top_indent, text.c_str(), (int32_t)text.size());
}

void draw_cursor(HDC dc, HPEN pen, const std::wstring &text, size_t cursor_position, const rect &position_)
{
    SelectObject(dc, pen);

    auto text_dimensions = calculate_text_dimensions(dc, text, cursor_position);
    auto cursor_coordinates = calculate_cursor_coordinates(text_dimensions.right, text_dimensions.bottom, position_);

    MoveToEx(dc, cursor_coordinates.left, cursor_coordinates.top, (LPPOINT)NULL);
    LineTo(dc, cursor_coordinates.right, cursor_coordinates.bottom);
}

#endif

void input::draw(graphic &gr)
{
    if (!showed_ || position_.width() == 0 || position_.height() == 0 || position_.width() <= input_horizontal_indent * 2)
    {
        return;
    }

#ifdef _WIN32
    SelectObject(gr.dc, !focused_ ? border_pen : focused_border_pen);
    SelectObject(gr.dc, background_brush);

    auto rnd = theme_dimension(theme_value::input_round, theme_);
    RoundRect(gr.dc, position_.left, position_.top, position_.right, position_.bottom, rnd, rnd);

    draw_text(gr.dc, text_, position_, theme_);

    draw_cursor(gr.dc, cursor_visible ? cursor_pen : background_pen, text_, cursor_position, position_);
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
            case mouse_event_type::move:

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

                            if (change_callback)
                            {
                                change_callback(text_);
                            }
                        }
                    break;
                    case vk_del:
                        if (!text_.empty())
                        {
                            text_.erase(cursor_position, 1);
                            redraw();

                            if (change_callback)
                            {
                                change_callback(text_);
                            }
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

                if (change_callback)
                {
                    change_callback(text_);
                }
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
