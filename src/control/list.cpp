//
// Copyright (c) 2021-2022 Anton Golovkov (udattsk at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/ud84/wui
//

#include <wui/control/list.hpp>

#include <wui/window/window.hpp>

#include <wui/theme/theme.hpp>

#include <wui/system/tools.hpp>

#include <algorithm>

namespace wui
{

list::list(std::shared_ptr<i_theme> theme__)
    : theme_(theme__),
    position_(),
    parent(),
    showed_(true), enabled_(true), focused_(false),
    columns(),
    item_height(0), item_count(0), selected_item_(0), start_item(0),
    draw_callback(),
    item_change_callback(),
    column_click_callback(),
    item_click_callback(),
    item_double_click_callback(),
    item_right_click_callback()
{
}

list::~list()
{
    auto parent_ = parent.lock();
    if (parent_)
    {
        parent_->remove_control(shared_from_this());
    }
}

void list::draw(graphic &gr, const rect &)
{
    if (!showed_)
    {
        return;
    }

    gr.draw_rect(position(),
        !focused_ ? theme_color(tc, tv_border, theme_) : theme_color(tc, tv_focused_border, theme_),
        theme_color(tc, tv_background, theme_),
        theme_dimension(tc, tv_border_width, theme_),
        theme_dimension(tc, tv_round, theme_));

    draw_titles(gr);

    draw_items(gr);

    draw_scrollbar(gr);
}

void list::receive_event(const event &)
{
    if (!showed_ || !enabled_)
    {
        return;
    }
}

void list::set_position(const rect &position__, bool redraw)
{
    update_control_position(position_, position__, showed_ && redraw, parent);
}

rect list::position() const
{
    return get_control_position(position_, parent);
}

void list::set_parent(std::shared_ptr<window> window)
{
    parent = window;
}

void list::clear_parent()
{
    parent.reset();
}

bool list::topmost() const
{
    return false;
}

void list::set_focus()
{
    if (enabled_ && showed_)
    {
        focused_ = true;

        redraw();
    }
}

bool list::remove_focus()
{
    focused_ = false;

    redraw();

    return true;
}

bool list::focused() const
{
    return focused_;
}

bool list::focusing() const
{
    return enabled_ && showed_;
}

void list::update_theme(std::shared_ptr<i_theme> theme__)
{
    if (theme_ && !theme__)
    {
        return;
    }
    theme_ = theme__;

    redraw();
}

void list::show()
{
    if (!showed_)
    {
        showed_ = true;
        redraw();
    }
}

void list::hide()
{
    if (showed_)
    {
        showed_ = false;
        auto parent_ = parent.lock();
        if (parent_)
        {
            parent_->redraw(position_, true);
        }
    }
}

bool list::showed() const
{
    return showed_;
}

void list::enable()
{
    enabled_ = true;
    redraw();
}

void list::disable()
{
    enabled_ = false;
    redraw();
}

bool list::enabled() const
{
    return enabled_;
}

void list::add_column(int32_t width, const std::string &caption)
{
    columns.emplace_back(column{ width, caption });
}

void list::select_item(int32_t n_item)
{
    selected_item_ = n_item;

    auto visible_item_count = get_visible_item_count();

    if (selected_item_ < start_item)
    {
        auto diff = start_item - selected_item_;
        for (auto i = 0; i != diff; ++i)
        {
            scroll_up();
        }
    }
    else if (selected_item_ > visible_item_count + start_item)
    {
        auto diff = selected_item_ + 1 - visible_item_count + start_item;
        for (auto i = 0; i != diff; ++i)
        {
            scroll_down();
        }
    }

    redraw();
}

int32_t list::selected_item() const
{
    return selected_item_;
}

void list::set_column_width(int32_t n_item, int32_t width)
{
    if (columns.size() > n_item)
    {
        columns[n_item].width = width;
        redraw();
    }
}

void list::set_item_height(int32_t height)
{
    item_height = height;
}

void list::set_item_count(int32_t count)
{
    item_count = count;
    redraw();
}

void list::set_draw_callback(std::function<void(graphic&, int32_t, const rect&, bool selected)> draw_callback_)
{
    draw_callback = draw_callback_;
}

void list::set_item_change_callback(std::function<void(int32_t)> item_change_callback_)
{
    item_change_callback = item_change_callback_;
}

void list::set_column_click_callback(std::function<void(int32_t)> column_click_callback_)
{
    column_click_callback = column_click_callback_;
}

void list::set_item_click_callback(std::function<void(int32_t)> item_click_callback_)
{
    item_click_callback = item_click_callback_;
}

void list::set_item_double_click_callback(std::function<void(int32_t)> item_double_click_callback_)
{
    item_double_click_callback = item_double_click_callback_;
}

void list::set_item_right_click_callback(std::function<void(int32_t)> item_right_click_callback_)
{
    item_right_click_callback = item_right_click_callback_;
}

void list::redraw()
{
    if (showed_)
    {
        auto parent_ = parent.lock();
        if (parent_)
        {
            parent_->redraw(position_);
        }
    }
}

void list::draw_titles(graphic &gr_)
{

}

void list::draw_items(graphic &gr_)
{

}

void list::draw_scrollbar(graphic &gr_)
{

}

int32_t list::get_title_height() const
{
    if (!columns.empty())
    {
        return 20;
    }

    return 0;
}

int32_t list::get_visible_item_count() const
{
    return static_cast<int32_t>(position_.height() / item_height);
}

void list::scroll_up()
{
    if (start_item == 0)
    {
        return;
    }

    --start_item;
    redraw();
}

void list::scroll_down()
{
    auto visible_item_count = get_visible_item_count();

    if (visible_item_count >= item_count || start_item == item_count - visible_item_count)
    {
        return;
    }

    ++start_item;
    redraw();
}

}
