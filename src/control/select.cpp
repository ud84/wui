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

#include <algorithm>

namespace wui
{

static const int32_t select_horizontal_indent = 5;

select::select(std::shared_ptr<i_theme> theme__)
    : items(),
    change_callback(),
    theme_(theme__),
    position_(),
    parent(),
    my_control_sid(), my_plain_sid(),
    list_theme(make_custom_theme()),
    list_(new list(list_theme)),
    showed_(true), enabled_(true), active(false),
    focused_(false),
    focusing_(true),
    left_shift(0)
{
    update_list_theme();

    list_->set_mode(list::list_mode::simple_topmost);
    list_->set_draw_callback(std::bind(&select::draw_list_item, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4, std::placeholders::_5));
    list_->set_item_activate_callback(std::bind(&select::activate_list_item, this, std::placeholders::_1));
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

    if (control_pos.width() < control_pos.height())
    {
        return;
    }

    /// Draw the button
    draw_arrow_down(gr, { control_pos.right - static_cast<int32_t>(control_pos.height() / 1.5),
            control_pos.top + static_cast<int32_t>(control_pos.height() / 2.1)});

    auto font_ = theme_font(tc, tv_font, theme_);

    if (static_cast<int32_t>(items.size()) <= list_->selected_item())
    {
        return;
    }

    auto text = items[list_->selected_item()].text;

    auto text_size = gr.measure_text(text, font_);

    truncate_line(text, gr, font_, control_pos.width() - control_pos.height(), 1);

    gr.draw_text({ control_pos.left + border_width + select_horizontal_indent,
        control_pos.top + (control_pos.height() - text_size.height()) / 2 },
        text,
        theme_color(tc, tv_text, theme_),
        font_);
}


void select::draw_arrow_down(graphic &gr, rect pos)
{
    auto color = theme_color(tc, !active ? tv_border : tv_focused_border, theme_);

    int w = 8, h = 4;

    for (int j = 0; j != h; ++j)
    {
        for (int i = 0; i != w; ++i)
        {
            gr.draw_pixel({ pos.left + j + i, pos.top + j }, color);
        }
        w -= 2;
    }
}

void select::select_up()
{
    if (!items.empty() && list_->selected_item() > 0)
    {
        list_->select_item(list_->selected_item() - 1);
        redraw();
    }
}

void select::select_down()
{
    if (!items.empty() && list_->selected_item() < static_cast<int32_t>(items.size()) - 1)
    {
        list_->select_item(list_->selected_item() + 1);
        redraw();
    }
}

void select::show_list()
{
    list_->set_position({ position_.left, position_.top, position_.right, position_.top + list_->get_item_height() * static_cast<int32_t>(items.size()) });
    auto pos = get_popup_position(parent, position(), list_->position(), 0);

    list_->set_position(pos, true);
    list_->show();
    
    auto parent_ = parent.lock();
    if (parent_)
    {
        parent_->set_focused(list_);
    }
}

void select::receive_control_events(const event &ev)
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
            case mouse_event_type::wheel:
                if (ev.mouse_event_.wheel_delta > 0)
                {
                    select_up();
                }
                else
                {
                    select_down();
                }
                redraw();
            break;
            case mouse_event_type::left_up:
                if (!list_->showed())
                {
                    show_list();
                }
                else
                {
                    list_->hide();
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
                    case vk_up:
                        select_up();
                    break;
                    case vk_down:
                        select_down();
                    break;
                    case vk_home: case vk_page_up:
                        if (!items.empty())
                        {
                            list_->select_item(0);
                            redraw();
                        }
                    break;
                    case vk_end: case vk_page_down:
                        if (!items.empty())
                        {
                            list_->select_item(static_cast<int32_t>(items.size() - 1));
                            redraw();
                        }
                    break;
                }
            break;
        }
    }
}

void select::receive_plain_events(const event &ev)
{
    if (!list_->showed())
    {
        return;
    }

    switch (ev.type)
    {
    case event_type::mouse:
        if (ev.mouse_event_.type == mouse_event_type::left_up &&
            !list_->position().in({ ev.mouse_event_.x, ev.mouse_event_.y, ev.mouse_event_.x, ev.mouse_event_.y }) &&
            !position().in({ ev.mouse_event_.x, ev.mouse_event_.y, ev.mouse_event_.x, ev.mouse_event_.y }))
        {
            list_->hide();
        }
        break;
    case event_type::keyboard:
        if (ev.keyboard_event_.type == keyboard_event_type::up)
        {
            if (ev.keyboard_event_.key[0] == vk_esc)
            {
                list_->hide();
            }
        }
        break;
    }
}

void select::update_list_theme()
{
    list_theme->load_theme(theme_ ? *theme_ : *get_default_theme());

    list_theme->set_color(list::tc, list::tv_background, theme_color(tc, tv_background, theme_));
    list_theme->set_color(list::tc, list::tv_border, theme_color(tc, tv_border, theme_));
    list_theme->set_color(list::tc, list::tv_focused_border, theme_color(tc, tv_border, theme_));
    list_theme->set_dimension(list::tc, list::tv_border_width, theme_dimension(tc, tv_border_width, theme_));
    list_theme->set_color(list::tc, list::tv_scrollbar, theme_color(tc, tv_scrollbar, theme_));
    list_theme->set_color(list::tc, list::tv_scrollbar_slider, theme_color(tc, tv_scrollbar_slider, theme_));
    list_theme->set_color(list::tc, list::tv_scrollbar_slider_acive, theme_color(tc, tv_scrollbar_slider_acive, theme_));
    list_theme->set_dimension(list::tc, list::tv_round, theme_dimension(tc, tv_round, theme_));
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

    if (window_)
    {
        window_->add_control(list_, { 0 });
        list_->hide();

        my_control_sid = window_->subscribe(std::bind(&select::receive_control_events, this, std::placeholders::_1),
            static_cast<event_type>(static_cast<uint32_t>(event_type::mouse) | static_cast<uint32_t>(event_type::keyboard)),
            shared_from_this());

        my_plain_sid = window_->subscribe(std::bind(&select::receive_plain_events, this, std::placeholders::_1),
            static_cast<event_type>(static_cast<uint32_t>(event_type::mouse) | static_cast<uint32_t>(event_type::keyboard)));
    }
}

void select::clear_parent()
{
    auto parent_ = parent.lock();
    if (parent_)
    {
        list_->clear_parent();
        
        parent_->unsubscribe(my_control_sid);
        my_control_sid = -1;

        parent_->unsubscribe(my_plain_sid);
        my_plain_sid = -1;
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

    update_list_theme();
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

void select::update_item(const select_item &si)
{
    auto it = std::find(items.begin(), items.end(), si.id);
    if (it != items.end())
    {
        *it = si;
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

void select::select_item_number(int32_t index)
{
    list_->select_item(index);
    redraw();
}

void select::select_item_id(int32_t id)
{
    for (int32_t i = 0; i != static_cast<int32_t>(items.size()); ++i)
    {
        if (items[i].id == id)
        {
            list_->select_item(i);
            redraw();
            break;
        }
    }
}

select_item select::selected_item() const
{
    auto item_number = list_->selected_item();

    select_item result;

    if (item_number != -1 && item_number < static_cast<int32_t>(items.size()))
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

void select::draw_list_item(graphic &gr, int32_t n_item, const rect &item_rect_, list::item_state state, const std::vector<list::column> &)
{
    if (static_cast<int32_t>(items.size()) <= n_item)
    {
        return;
    }

    auto item = items[n_item];
    
    auto border_width = theme_dimension(tc, tv_border_width);

    auto item_rect = item_rect_;

    if (item_rect.bottom > list_->position().bottom - border_width)
    {
        item_rect.bottom = list_->position().bottom - border_width;
    }

    if (state == wui::list::item_state::active)
    {
        gr.draw_rect(item_rect, wui::theme_color(tc, tv_active_item));
    }
    else if (state == wui::list::item_state::selected)
    {
        gr.draw_rect(item_rect, wui::theme_color(tc, tv_selected_item));
    }

    auto text_color = theme_color(tc, tv_text);
    auto font = theme_font(tc, tv_font);

    auto text = items[n_item].text;

    auto text_size = gr.measure_text(text, font);

    truncate_line(text, gr, font, item_rect_.width() - item_rect_.height(), 1);

    auto text_height = text_size.height();
    if (text_height <= item_rect.height())
    {
        gr.draw_text({ item_rect.left + select_horizontal_indent, item_rect_.top + (item_rect_.height() - text_height) / 2 }, text, text_color, font);
    }
}

void select::activate_list_item(int32_t n_item)
{
    list_->hide();
    redraw();
}

}
