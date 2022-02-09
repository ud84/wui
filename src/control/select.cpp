//
// Copyright (c) 2021-2022 Anton Golovkov (udattsk at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/ud84/wui
//

#include <wui/control/select.hpp>

#include <wui/control/list.hpp>

#include <wui/window/window.hpp>

#include <wui/theme/theme.hpp>

#include <wui/system/tools.hpp>

#include <boost/nowide/convert.hpp>
#include <utf8/utf8.h>

namespace wui
{

select::select(std::shared_ptr<i_theme> theme__)
    : items(),
    change_callback(),
    theme_(theme__),
    position_(),
    parent(),
    my_subscriber_id(),
    showed_(true), enabled_(true), active(false),
    focused_(false),
    focusing_(true),
    left_shift(0)
{
}

select::~select()
{
    auto parent_ = parent.lock();
    if (parent_)
    {
        parent_->remove_control(shared_from_this());
    }
}

void select::draw(graphic &gr, const rect &)
{
    if (!showed_ || position_.width() == 0 || position_.height() == 0)
    {
        return;
    }

    auto control_pos = position();
    auto border_width = theme_dimension(tc, tv_border_width, theme_);

    /// Draw the frame
    gr.draw_rect(control_pos,
        !focused_ ? theme_color(tc, tv_border, theme_) : theme_color(tc, tv_focused_border, theme_),
        theme_color(tc, tv_background, theme_),
        border_width,
        theme_dimension(tc, tv_round, theme_));

    /// Draw the button
    gr.draw_rect({ control_pos.right - control_pos.height() - border_width,
            control_pos.top + border_width,
            control_pos.right - border_width,
            control_pos.bottom - border_width },
        theme_color(tc, active ? tv_button_active : tv_button_calm, theme_));

    auto font_ = theme_font(tc, tv_font, theme_);
}

void select::receive_event(const event &ev)
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
                active = true;
                redraw();
            break;
            case mouse_event_type::leave:
                active = false;
                redraw();
            break;
            case mouse_event_type::left_down:
                

                redraw();
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
                switch (ev.keyboard_event_.key[0])
                {
                    case vk_left:
                        
                    break;
                    case vk_right:
                        
                    break;
                    case vk_home:
                        
                    break;
                    case vk_end:

                    break;
                }
            break;
            case keyboard_event_type::up:
                //ev.keyboard_event_.key[0] == vk_shift
            break;
        }
    }
}

void select::set_position(const rect &position__, bool redraw)
{
    update_control_position(position_, position__, showed_ && redraw, parent);
}

rect select::position() const
{
    return get_control_position(position_, parent);
}

void select::set_parent(std::shared_ptr<window> window_)
{
    parent = window_;
    my_subscriber_id = window_->subscribe(std::bind(&select::receive_event, this, std::placeholders::_1),
        static_cast<event_type>(static_cast<uint32_t>(event_type::mouse) | static_cast<uint32_t>(event_type::keyboard)),
        shared_from_this());
}

void select::clear_parent()
{
    auto parent_ = parent.lock();
    if (parent_)
    {
        parent_->unsubscribe(my_subscriber_id);
    }
    parent.reset();
}

bool select::topmost() const
{
    return false;
}

void select::set_focus()
{
    if (focusing_ && enabled_ && showed_)
    {
        focused_ = true;

        redraw();
    }
}

bool select::remove_focus()
{
    focused_ = false;

    redraw();

    return true;
}

bool select::focused() const
{
    return focused_;
}

bool select::focusing() const
{
    return enabled_ && showed_ && focusing_;
}

void select::update_theme(std::shared_ptr<i_theme> theme__)
{
    if (theme_ && !theme__)
    {
        return;
    }
    theme_ = theme__;
}

void select::show()
{
    showed_ = true;
    redraw();
}

void select::hide()
{
    showed_ = false;
    auto parent_ = parent.lock();
    if (parent_)
    {
        parent_->redraw(position(), true);
    }
}

bool select::showed() const
{
    return showed_;
}

void select::enable()
{
    enabled_ = true;
    redraw();
}

void select::disable()
{
    enabled_ = false;
    redraw();
}

bool select::enabled() const
{
    return enabled_;
}

void select::set_items(const select_items_t &items_)
{
    items = items_;
    list_->set_item_count(static_cast<int32_t>(items.size()));
}

void select::update_item(const select_item &mi)
{
    auto it = std::find(items.begin(), items.end(), mi.id);
    if (it != items.end())
    {
        *it = mi;
    }
    list_->set_item_count(static_cast<int32_t>(items.size()));
}

void select::swap_items(int32_t first_item_id, int32_t second_item_id)
{
    auto first_it = std::find(items.begin(), items.end(), first_item_id);
    if (first_it != items.end())
    {
        auto second_it = std::find(items.begin(), items.end(), second_item_id);
        if (second_it != items.end())
        {
            std::swap(*first_it, *second_it);
        }
    }
    list_->set_item_count(static_cast<int32_t>(items.size()));
}

void select::delete_item(int32_t id)
{
    auto it = std::find(items.begin(), items.end(), id);
    if (it != items.end())
    {
        items.erase(it);
    }
    list_->set_item_count(static_cast<int32_t>(items.size()));
}

void select::set_item_height(int32_t item_height_)
{
    list_->set_item_height(item_height_);
}

select_item select::selected_item() const
{
    auto item_number = list_->selected_item();

    select_item result;

    if (item_number != -1 && item_number < items.size())
    {
        result = items[item_number];
    }

    return result;
}

void select::set_change_callback(std::function<void(int32_t, const std::string&)> change_callback_)
{
    change_callback = change_callback_;
}

void select::redraw()
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
