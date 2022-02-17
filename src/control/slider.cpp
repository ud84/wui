//
// Copyright (c) 2021-2022 Anton Golovkov (udattsk at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/ud84/wui
//

#include <wui/control/slider.hpp>

#include <wui/window/window.hpp>

#include <wui/theme/theme.hpp>

#include <wui/system/tools.hpp>

#include <cmath>

namespace wui
{

slider::slider(int32_t from_, int32_t to_, int32_t value_, std::function<void(int32_t)> change_callback_, slider_orientation orientation_, std::shared_ptr<i_theme> theme__)
    : orientation(orientation_),
    from(from_), to(to_), value(value_),
    change_callback(change_callback_),
    theme_(theme__),
    position_(),
    parent(),
    my_control_sid(), my_plain_sid(),
    showed_(true), enabled_(true), active(false), focused_(false),
    slider_scrolling(false), mouse_on_control(false),
    slider_position({ 0 }),
    diff_size(0.)
{
}

slider::~slider()
{
    auto parent_ = parent.lock();
    if (parent_)
    {
        parent_->remove_control(shared_from_this());
    }
}

void slider::draw(graphic &gr, const rect &)
{
    if (!showed_)
    {
        return;
    }

    auto control_pos = position();

    auto slider_width = theme_dimension(tc, tv_slider_width, theme_);
    auto slider_height = theme_dimension(tc, tv_slider_height, theme_);
    auto slider_round = theme_dimension(tc, tv_slider_round, theme_);

    double total = (orientation == slider_orientation::horizontal ? control_pos.width() : control_pos.height()) - slider_width / 2;
    double slider_pos = (total * static_cast<double>(value)) / static_cast<double>(to - from);

    if (slider_pos < slider_width / 2)
    {
        slider_pos = slider_width / 2;
    }

    auto perform_color = theme_color(tc, tv_perform, theme_);
    auto remain_color = theme_color(tc, active || focused_ ? tv_active : tv_remain, theme_);

    auto slider_color = active || focused_ ? remain_color : perform_color;

    if (orientation == slider_orientation::horizontal)
    {
        auto center = control_pos.top + (control_pos.height() / 2) - 1;
        gr.draw_rect({ control_pos.left, center - 1, control_pos.left + static_cast<int32_t>(slider_pos), center + 1 }, perform_color);
        gr.draw_rect({ control_pos.left + static_cast<int32_t>(slider_pos), center - 1, control_pos.right, center + 1 }, remain_color);

        slider_position = { control_pos.left + static_cast<int32_t>(slider_pos) - (slider_width / 2),
                center - (slider_height / 2) + 1,
                control_pos.left + static_cast<int32_t>(slider_pos) + (slider_width / 2),
                center + (slider_height / 2) - 1 };

        gr.draw_rect(slider_position,
            slider_color,
            slider_color,
            1,
            slider_round);
    }
    else if (orientation == slider_orientation::vertical)
    {
        auto center = control_pos.left + (control_pos.width() / 2) - 1;
        gr.draw_rect({ center - 1, control_pos.bottom, center + 1, control_pos.bottom - static_cast<int32_t>(slider_pos) }, perform_color);
        gr.draw_rect({ center - 1, control_pos.top, center + 1, control_pos.bottom - static_cast<int32_t>(slider_pos) }, remain_color);

        slider_position = { center - (slider_height / 2) + 1,
                control_pos.bottom - static_cast<int32_t>(slider_pos) - (slider_width / 2),
                center + (slider_height / 2) - 1,
                control_pos.bottom - static_cast<int32_t>(slider_pos) + (slider_width / 2) };

        gr.draw_rect(slider_position,
            slider_color,
            slider_color,
            1,
            slider_round);
    }
}

void slider::receive_control_events(const event &ev)
{
    if (!showed_ || !enabled_)
    {
        return;
    }

    if (ev.type == event_type::mouse)
    {
        switch (ev.mouse_event_.type)
        {
            case mouse_event_type::enter:
            {
                mouse_on_control = true;
                if (!slider_scrolling)
                {
                    active = true;
                    redraw();
                }
            }
            break;
            case mouse_event_type::leave:
                mouse_on_control = false;
                if (!slider_scrolling)
                {
                    active = false;
                    redraw();
                }
            break;
            case mouse_event_type::left_down:
                if (slider_position.in(ev.mouse_event_.x, ev.mouse_event_.y))
                {
                    slider_scrolling = true;
                }
                else
                {
                    move_slider(ev.mouse_event_.x, ev.mouse_event_.y);
                }
            break;
            case mouse_event_type::left_up:                
                active = false;
                slider_scrolling = false;
            break;
            case mouse_event_type::move:
                if (slider_scrolling)
                {
                    move_slider(ev.mouse_event_.x, ev.mouse_event_.y);
                }
            break;
            case mouse_event_type::wheel:
                if (ev.mouse_event_.wheel_delta > 0)
                {
                    scroll_up();
                }
                else
                {
                    scroll_down();
                }
            break;
        }
    }
    else if (ev.type == event_type::keyboard)
    {
        switch (ev.keyboard_event_.type)
        {
            case keyboard_event_type::down:
                switch (ev.keyboard_event_.key[0])
                {
                    case vk_end: case vk_page_up:
                    {
                        value = to;
                        redraw();
                        if (change_callback)
                        {
                            change_callback(value);
                        }
                    }
                    break;
                    case vk_home: case vk_page_down:
                    {
                        value = from;
                        redraw();
                        if (change_callback)
                        {
                            change_callback(value);
                        }
                    }
                    break;
                    case vk_up: case vk_right:
                        if (value != to)
                        {
                            scroll_up();
                        }
                    break;
                    case vk_down: case vk_left:
                        if (value != from)
                        {
                            scroll_down();
                        }
                    break;
                }
            break;
        }
    }
}

void slider::receive_plain_events(const event &ev)
{
    if (!showed_ || !enabled_)
    {
        return;
    }

    if (!mouse_on_control && ev.type == event_type::mouse)
    {
        switch (ev.mouse_event_.type)
        {
            case mouse_event_type::move:
                if (slider_scrolling)
                {
                    move_slider(ev.mouse_event_.x, ev.mouse_event_.y);
                }
            break;
            case mouse_event_type::left_up:
                slider_scrolling = false;
            break;
        }
    }
}

void slider::set_position(const rect &position__, bool redraw)
{
    update_control_position(position_, position__, showed_ && redraw, parent);

    calc_consts();
}

rect slider::position() const
{
    return get_control_position(position_, parent);
}

void slider::set_parent(std::shared_ptr<window> window_)
{
    parent = window_;

    my_control_sid = window_->subscribe(std::bind(&slider::receive_control_events, this, std::placeholders::_1),
        static_cast<event_type>(static_cast<uint32_t>(event_type::keyboard) | static_cast<uint32_t>(event_type::mouse)),
        shared_from_this());

    my_plain_sid = window_->subscribe(std::bind(&slider::receive_plain_events, this, std::placeholders::_1), event_type::mouse);
}

void slider::clear_parent()
{
    auto parent_ = parent.lock();
    if (parent_)
    {
        parent_->unsubscribe(my_control_sid);
        parent_->unsubscribe(my_plain_sid);
    }
    parent.reset();
}

bool slider::topmost() const
{
    return false;
}

void slider::set_focus()
{
    if (enabled_ && showed_)
    {
        focused_ = true;

        redraw();
    }
}

bool slider::remove_focus()
{
    focused_ = false;

    redraw();

    return true;
}

bool slider::focused() const
{
    return focused_;
}

bool slider::focusing() const
{
    return enabled_ && showed_;
}

void slider::update_theme(std::shared_ptr<i_theme> theme__)
{
    if (theme_ && !theme__)
    {
        return;
    }
    theme_ = theme__;

    redraw();
}

void slider::show()
{
    if (!showed_)
    {
        showed_ = true;
        redraw();
    }
}

void slider::hide()
{
    if (showed_)
    {
        showed_ = false;
        auto parent_ = parent.lock();
        if (parent_)
        {
            parent_->redraw(position(), true);
        }
    }
}

bool slider::showed() const
{
    return showed_;
}

void slider::enable()
{
    enabled_ = true;
    redraw();
}

void slider::disable()
{
    enabled_ = false;
    redraw();
}

bool slider::enabled() const
{
    return enabled_;
}

void slider::set_range(int32_t from_, int32_t to_)
{
    from = from_;
    to = to_;

    calc_consts();

    redraw();
}

void slider::set_value(int32_t value_)
{
    value = value_;
    redraw();
}

void slider::set_callback(std::function<void(int32_t)> change_callback_)
{
    change_callback = change_callback_;
}

void slider::redraw()
{
    if (showed_)
    {
        auto parent_ = parent.lock();
        if (parent_)
        {
            parent_->redraw(position());
        }
    }
}

void slider::calc_consts()
{
    diff_size = static_cast<double>(to - from) / static_cast<double>(orientation == slider_orientation::horizontal ? position_.width() : position_.height());
}

void slider::move_slider(int32_t x, int32_t y)
{
    if (orientation == slider_orientation::horizontal)
    {
        value = static_cast<int32_t>((x - position().left) * diff_size);
    }
    else
    {
        value = static_cast<int32_t>((position().bottom - y) * diff_size);
    }

    if (value > to)
    {
        value = to;
    }
    else if (value < from)
    {
        value = from;
    }

    redraw();

    if (change_callback)
    {
        change_callback(value);
    }
}

void slider::scroll_up()
{
    if (value == to)
    {
        return;
    }

    value += static_cast<int32_t>(round(diff_size));
    if (value > to)
    {
        value = to;
    }

    redraw();

    if (change_callback)
    {
        change_callback(value);
    }
}

void slider::scroll_down()
{
    if (value == from)
    {
        return;
    }

    value -= static_cast<int32_t>(round(diff_size));
    if (value < from)
    {
        value = from;
    }

    redraw();

    if (change_callback)
    {
        change_callback(value);
    }
}

}
