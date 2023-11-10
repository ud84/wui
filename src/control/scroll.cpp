//
// Copyright (c) 2021-2022 Anton Golovkov (udattsk at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/ud84/wui
//

#include <wui/control/scroll.hpp>

#include <wui/window/window.hpp>

#include <wui/theme/theme.hpp>

#include <wui/system/tools.hpp>

#include <boost/nowide/convert.hpp>

#include <cstring>

namespace wui
{

scroll::scroll(int32_t area_, int32_t scroll_pos_, double scroll_interval_,
    orientation orientation__,
    std::function<void(scroll_state, int32_t)> callback_,
    std::string_view theme_control_name, std::shared_ptr<i_theme> theme__)
    : tcn(theme_control_name),
    theme_(theme__),
    position_(),
    parent_(),
    showed_(true), topmost_(false),
    area(area_),
    scroll_pos(scroll_pos_),
    scroll_interval(scroll_interval_),
    orientation_(orientation__),
    callback(callback_),
    worker_action_(worker_action::undefined),
    worker(),
    worker_runned(false),
    scrollbar_state_(scrollbar_state::full),
    slider_scrolling(false),
    slider_click_pos(0),
    prev_scroll_pos(0)
{
}

scroll::~scroll()
{
    auto parent__ = parent_.lock();
    if (parent__)
    {
        parent__->remove_control(shared_from_this());
    }
}

void scroll::draw(graphic &gr, const rect &)
{
    if (!showed_)
    {
        return;
    }

    auto control_pos = position();

    if (orientation_ == orientation::vertical)
    {
        draw_vert_scrollbar(gr);
    }
    else
    {
        //draw_hor_scrollbar(gr);
    }
}

void scroll::set_position(const rect &position__, bool redraw)
{
    update_control_position(position_, position__, showed_ && redraw, parent_);
}

rect scroll::position() const
{
    return get_control_position(position_, parent_);
}

void scroll::set_parent(std::shared_ptr<window> window)
{
    parent_ = window;
}

std::weak_ptr<window> scroll::parent() const
{
    return parent_;
}

void scroll::clear_parent()
{
    parent_.reset();
}

void scroll::set_topmost(bool yes)
{
    topmost_ = yes;
}

bool scroll::topmost() const
{
    return topmost_;
}

bool scroll::focused() const
{
    return false;
}

bool scroll::focusing() const
{
    return false;
}

error scroll::get_error() const
{
    return {};
}

void scroll::update_theme_control_name(std::string_view theme_control_name)
{
    tcn = theme_control_name;
    update_theme(theme_);
}

void scroll::update_theme(std::shared_ptr<i_theme> theme__)
{
    if (theme_ && !theme__)
    {
        return;
    }
    theme_ = theme__;

    redraw();
}

void scroll::show()
{
    showed_ = true;
    redraw();
}

void scroll::hide()
{
    showed_ = false;
    auto parent__ = parent_.lock();
    if (parent__)
    {
        parent__->redraw(position(), true);
    }
}

bool scroll::showed() const
{
    return showed_;
}

void scroll::enable()
{
}

void scroll::disable()
{
}

bool scroll::enabled() const
{
    return true;
}

void scroll::set_values(int32_t area_, double scroll_interval_)
{
    area = area_;
    scroll_interval = scroll_interval_;

    redraw();
}

void scroll::set_scroll_pos(int32_t scroll_pos_)
{
    scroll_pos = scroll_pos_;

    redraw();
}

int32_t scroll::get_scroll_pos() const
{
    return scroll_pos;
}

void scroll::redraw()
{
    if (showed_)
    {
        auto parent__ = parent_.lock();
        if (parent__)
        {
            parent__->redraw(position());
        }
    }
}

void scroll::draw_vert_scrollbar(graphic& gr)
{
    rect bar_rect = { 0 }, top_button_rect = { 0 }, bottom_button_rect = { 0 }, slider_rect = { 0 };
    calc_scrollbar_params(&bar_rect, &top_button_rect, &bottom_button_rect, &slider_rect);

    if (slider_rect.bottom == 0)
    {
        return;
    }

    gr.draw_rect(bar_rect, theme_color(tcn, tv_background, theme_));

    gr.draw_rect(top_button_rect, theme_color(tcn, tv_slider, theme_));
    if (scrollbar_state_ == scrollbar_state::full)
    {
        draw_arrow_up(gr, top_button_rect);
    }

    gr.draw_rect(bottom_button_rect, theme_color(tcn, tv_slider, theme_));
    if (scrollbar_state_ == scrollbar_state::full)
    {
        draw_arrow_down(gr, bottom_button_rect);
    }

    gr.draw_rect(slider_rect, theme_color(tcn, scrollbar_state_ == scrollbar_state::full ? tv_slider_acive : tv_slider, theme_));
}

void scroll::draw_arrow_up(graphic& gr, rect button_pos)
{
    auto color = theme_color(tcn, tv_slider_acive, theme_);

    int w = 8, h = 4;

    for (int j = 0; j != h; ++j)
    {
        for (int i = 0; i != w; ++i)
        {
            gr.draw_pixel({ button_pos.left + 3 + j + i, button_pos.top + 8 - j }, color);
        }
        w -= 2;
    }
}

void scroll::draw_arrow_down(graphic& gr, rect button_pos)
{
    auto color = theme_color(tcn, tv_slider_acive, theme_);

    int w = 8, h = 4;

    for (int j = 0; j != h; ++j)
    {
        for (int i = 0; i != w; ++i)
        {
            gr.draw_pixel({ button_pos.left + 3 + j + i, button_pos.top + 5 + j }, color);
        }
        w -= 2;
    }
}

double scroll::get_scroll_interval() const
{
    return scroll_interval;
}

void scroll::move_slider(int32_t y)
{
    if (scroll_interval < 0)
    {
        return;
    }

    auto pos = y - position_.top - full_scrollbar_width - slider_click_pos;

    pos = pos * static_cast<int32_t>(scroll_interval);
    if (scroll_pos < 0)
    {
        scroll_pos = 0;
    }

    if (scroll_pos > area)
    {
        scroll_pos = area;
    }

    if (callback && prev_scroll_pos != scroll_pos)
    {
        if (scroll_pos == 0)
        {
            callback(scroll_state::up_end, scroll_pos);
        }
        else if (scroll_pos == area)
        {
            callback(scroll_state::down_end, scroll_pos);
        }
    }

    redraw();

    prev_scroll_pos = scroll_pos;
}

void scroll::scroll_up()
{
    if (scroll_pos == 0)
    {
        return;
    }

    auto scroll_interval = get_scroll_interval();
    if (scroll_interval < 0)
    {
        return;
    }

    scroll_pos -= static_cast<int32_t>(scroll_interval) * 10;
    if (scroll_pos < 0)
    {
        scroll_pos = 0;
    }

    redraw();

    if (scroll_pos == 0 && callback)
    {
        callback(scroll_state::up_end, scroll_pos);
    }
}

void scroll::scroll_down()
{
    auto scroll_interval = get_scroll_interval();
    if (scroll_interval < 0)
    {
        return;
    }

    if (area >= scroll_pos && area - scroll_pos > position_.height())
    {
        scroll_pos += static_cast<int32_t>(scroll_interval) * 10;
        redraw();
    }

    if (callback && area - scroll_pos <= position_.height() && prev_scroll_pos != scroll_pos)
    {
        callback(scroll_state::down_end, scroll_pos);
    }

    prev_scroll_pos = scroll_pos;
}

void scroll::calc_scrollbar_params(rect* bar_rect, rect* top_button_rect, rect* bottom_button_rect, rect* slider_rect)
{
    int32_t scrollbar_width = 0;
    if (scrollbar_state_ == scrollbar_state::tiny)
    {
        scrollbar_width = tiny_scrollbar_width;
    }
    else if (scrollbar_state_ == scrollbar_state::full)
    {
        scrollbar_width = full_scrollbar_width;
    }
    else
    {
        return;
    }

    const int32_t SB_WIDTH = scrollbar_width,
        SB_HEIGHT = full_scrollbar_width, SB_SILDER_MIN_WIDTH = 5,
        SB_BUTTON_WIDTH = SB_WIDTH, SB_BUTTON_HEIGHT = SB_HEIGHT;

    auto control_pos = position();
    
    double client_height = control_pos.height() - (SB_HEIGHT * 2);

    auto scroll_interval = get_scroll_interval();
    if (scroll_interval < 0)
    {
        return;
    }

    if (bar_rect)
    {
        *bar_rect = { control_pos.right, control_pos.top, control_pos.right + SB_WIDTH, control_pos.bottom };
    }

    if (top_button_rect && scrollbar_state_ == scrollbar_state::full)
    {
        *top_button_rect = { control_pos.right, control_pos.top, control_pos.right + SB_BUTTON_WIDTH, control_pos.top + SB_BUTTON_HEIGHT };
    }

    if (bottom_button_rect && scrollbar_state_ == scrollbar_state::full)
    {
        *bottom_button_rect = { control_pos.right, control_pos.bottom - SB_BUTTON_HEIGHT, control_pos.right + SB_BUTTON_WIDTH, control_pos.bottom };
    }

    if (slider_rect)
    {
        auto slider_top = control_pos.top + static_cast<int32_t>(round(((double)scroll_pos) / scroll_interval));
        auto slider_height = static_cast<int32_t>(round((client_height) / scroll_interval));

        if (slider_height < SB_SILDER_MIN_WIDTH)
        {
            slider_height = SB_SILDER_MIN_WIDTH;
        }

        *slider_rect = { control_pos.right,
            SB_HEIGHT + slider_top,
            control_pos.right + SB_BUTTON_WIDTH,
            SB_HEIGHT + slider_top + slider_height };

        if (scrollbar_state_ == scrollbar_state::full && bottom_button_rect && slider_rect->bottom > bottom_button_rect->top)
        {
            slider_rect->move(0, bottom_button_rect->top - slider_rect->bottom);
        }
    }
}


}
