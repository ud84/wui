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
    showed_(true), enabled_(true), active(false), focused_(false)
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

    double total = orientation == slider_orientation::horizontal ? control_pos.width() : control_pos.height();
    double slider_pos = (total * static_cast<double>(value)) / static_cast<double>(to - from);

    auto perform_color = theme_color(tc, tv_perform, theme_);
    auto remain_color = theme_color(tc, active || focused_ ? tv_active : tv_remain, theme_);

    auto slider_width = theme_dimension(tc, tv_slider_width, theme_);
    auto slider_height = theme_dimension(tc, tv_slider_height, theme_);
    auto slider_round = theme_dimension(tc, tv_slider_round, theme_);

    auto slider_color = focused_ ? remain_color : perform_color;

    if (orientation == slider_orientation::horizontal)
    {
        auto center = control_pos.top + (control_pos.height() / 2) - 1;
        gr.draw_rect({ control_pos.left, center - 1, control_pos.left + static_cast<int32_t>(slider_pos), center + 1 }, perform_color);
        gr.draw_rect({ control_pos.left + static_cast<int32_t>(slider_pos), center - 1, control_pos.right, center + 1 }, remain_color);

        gr.draw_rect({ control_pos.left + static_cast<int32_t>(slider_pos) - (slider_width / 2),
                center - (slider_height / 2),
                control_pos.left + static_cast<int32_t>(slider_pos) + (slider_width / 2),
                center + (slider_height / 2) },
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

        gr.draw_rect({ center - (slider_height / 2),
                control_pos.bottom - static_cast<int32_t>(slider_pos) - (slider_width / 2),
                center + (slider_height / 2),
                control_pos.bottom - static_cast<int32_t>(slider_pos) + (slider_width / 2) },
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
                active = true;
                auto parent_ = parent.lock();
                if (parent_)
                {
                    set_cursor(parent_->context(), cursor::default_);
                }
                redraw();

                
            }
            break;
            case mouse_event_type::leave:
                active = false;
                redraw();
            break;
            case mouse_event_type::left_up:                
                active = false;

                if (change_callback && enabled_)
                {
                    //change_callback();
                }
            break;
        }
    }
    else if (ev.type == event_type::internal)
    {
        if (ev.internal_event_.type == internal_event_type::execute_focused && change_callback)
        {
            //change_callback();
        }
    }
}

void slider::receive_plain_events(const event &ev)
{
    if (!showed_ || !enabled_)
    {
        return;
    }
}

void slider::set_position(const rect &position__, bool redraw)
{
    update_control_position(position_, position__, showed_ && redraw, parent);
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

}
