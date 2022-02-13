//
// Copyright (c) 2021-2022 Anton Golovkov (udattsk at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/ud84/wui
//

#include <wui/control/splitter.hpp>

#include <wui/window/window.hpp>

#include <wui/theme/theme.hpp>

#include <wui/system/tools.hpp>

namespace wui
{

 splitter::splitter(splitter_orientation orientation_, std::function<void(int32_t, int32_t)> callback_, std::shared_ptr<i_theme> theme__)
    : orientation(orientation_),
    callback(callback_),
    theme_(theme__),
    position_(),
    parent(),
    my_control_sid(), my_plain_sid(),
    showed_(true), enabled_(true), active(false)
{
}

splitter::~splitter()
{
    auto parent_ = parent.lock();
    if (parent_)
    {
        parent_->remove_control(shared_from_this());
    }
}

void splitter::draw(graphic &gr, const rect &)
{
    if (!showed_)
    {
        return;
    }
    
    gr.draw_rect(position(), theme_color(tc, active ? tv_active : tv_calm, theme_));
}

void splitter::receive_control_events(const event &ev)
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
                auto parent_ = parent.lock();
                if (parent_)
                {
                    if (orientation == splitter_orientation::vertical)
                    {
                        set_cursor(parent_->context(), cursor::size_we);
                    }
                    else if (orientation == splitter_orientation::horizontal)
                    {
                        set_cursor(parent_->context(), cursor::size_ns);
                    }
                }
            }
            break;
            case mouse_event_type::leave:
            {
                if (!active)
                {
                    auto parent_ = parent.lock();
                    if (parent_)
                    {
                        set_cursor(parent_->context(), cursor::default_);
                    }
                }
            }
            break;
            case mouse_event_type::left_down:
                active = true;
                redraw();
            break;
        }
    }
}

void splitter::receive_plain_events(const event &ev)
{
    switch (ev.mouse_event_.type)
    {
        case mouse_event_type::move:
        if (active)
        {
            auto pos = position_;

            if (orientation == splitter_orientation::vertical)
            {
                pos.put(ev.mouse_event_.x, 0);
            }
            else if (orientation == splitter_orientation::horizontal)
            {
                pos.put(0, ev.mouse_event_.y);
            }

            set_position(pos, true);

            if (callback)
            {
                callback(ev.mouse_event_.x, ev.mouse_event_.y);
            }
        }
        break;
        case mouse_event_type::left_up:
            if (active)
            {
                active = false;

                auto parent_ = parent.lock();
                if (parent_)
                {
                   set_cursor(parent_->context(), cursor::default_);
                }

                redraw();
            }
        break;
    }
}

void splitter::set_position(const rect &position__, bool redraw)
{
    update_control_position(position_, position__, showed_ && redraw, parent);
}

rect splitter::position() const
{
    return get_control_position(position_, parent);
}

void splitter::set_parent(std::shared_ptr<window> window_)
{
    parent = window_;
    
    my_control_sid = window_->subscribe(std::bind(&splitter::receive_control_events, this, std::placeholders::_1), event_type::mouse, shared_from_this());
    my_plain_sid = window_->subscribe(std::bind(&splitter::receive_plain_events, this, std::placeholders::_1), event_type::mouse);
}

void splitter::clear_parent()
{
    auto parent_ = parent.lock();
    if (parent_)
    {
        parent_->unsubscribe(my_control_sid);
        parent_->unsubscribe(my_plain_sid);
    }
    parent.reset();
}

bool splitter::topmost() const
{
    return false;
}

void splitter::set_focus()
{
}

bool splitter::remove_focus()
{
    return true;
}

bool splitter::focused() const
{
    return false;
}

bool splitter::focusing() const
{
    return false;
}

void splitter::update_theme(std::shared_ptr<i_theme> theme__)
{
    if (theme_ && !theme__)
    {
        return;
    }
    theme_ = theme__;

    redraw();
}

void splitter::show()
{
    if (!showed_)
    {
        showed_ = true;
        redraw();
    }
}

void splitter::hide()
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

bool splitter::showed() const
{
    return showed_;
}

void splitter::enable()
{
    enabled_ = true;
    redraw();
}

void splitter::disable()
{
    enabled_ = false;
    redraw();
}

bool splitter::enabled() const
{
    return enabled_;
}

void splitter::set_callback(std::function<void(int32_t, int32_t)> callback_)
{
    callback = callback_;
}

void splitter::redraw()
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
