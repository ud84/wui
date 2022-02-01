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
    my_subscriber_id(-1),
    showed_(true), enabled_(true), focused_(false),
    columns(),
    item_height(0), item_count(0), selected_item_(0), start_item(0),
    timer_action_(timer_action::undefined),
    timer_(std::bind([this]() { /* todo */ })),
    mouse_on_control(false), mouse_on_scrollbar(false),
    slider_scrolling(false),
    prev_scroll_pos(0),
    title_height(-1),
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

void list::receive_event(const event &ev)
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
                mouse_on_control = true;
                redraw();
            break;
            case mouse_event_type::leave:
                mouse_on_control = false;
                redraw();
            break;
            case mouse_event_type::left_up:
                
            break;
            case mouse_event_type::move:
                if (ev.mouse_event_.x > position().right - full_scrollbar_width)
                {
                    if (!mouse_on_scrollbar)
                    {
                        mouse_on_scrollbar = true;
                        redraw();
                    }
                }
                else
                {
                    if (mouse_on_scrollbar)
                    {
                        mouse_on_scrollbar = false;
                        redraw();
                    }
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
    else if (ev.type == event_type::internal)
    {
        if (ev.internal_event_.type == internal_event_type::execute_focused && item_double_click_callback)
        {
            item_double_click_callback(selected_item_);
        }
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
    my_subscriber_id = window->subscribe(std::bind(&list::receive_event, this, std::placeholders::_1),
        static_cast<event_type>(static_cast<uint32_t>(event_type::internal) | static_cast<uint32_t>(event_type::mouse) | static_cast<uint32_t>(event_type::keyboard)),
        shared_from_this());
}

void list::clear_parent()
{
    auto parent_ = parent.lock();
    if (parent_)
    {
        parent_->unsubscribe(my_subscriber_id);
    }
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
            parent_->redraw(position(), true);
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

void list::update_columns(const std::vector<column> &columns_)
{
    columns = columns_;
    title_height = -1;
    redraw();
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

void list::set_draw_callback(std::function<void(graphic&, int32_t, const rect&, bool selected, const std::vector<column> &columns)> draw_callback_)
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
            parent_->redraw(position());
        }
    }
}

void list::draw_titles(graphic &gr_)
{
    auto font = theme_font(tc, tv_font, theme_);
    auto text_indent = theme_dimension(tc, tv_text_indent, theme_);

    if (title_height == -1)
    {
        if (columns.empty())
        {
            title_height = 0;
            return;
        }

        auto text_dimensions = gr_.measure_text("Qq", font);

        title_height = text_dimensions.width() + text_indent * 2;
    }

    auto control_position = position();
    
    int32_t top = control_position.top + theme_dimension(tc, tv_border_width, theme_),
        pos = control_position.left + theme_dimension(tc, tv_border_width, theme_);

    auto title_color = theme_color(tc, tv_title, theme_);
    auto title_text_color = theme_color(tc, tv_title_text, theme_);

    for (auto &c : columns)
    {
        gr_.draw_rect({ pos, top, pos + c.width - 1, top + title_height }, title_color);
        gr_.draw_text({ pos + text_indent, top + text_indent, 0, 0 }, c.caption, title_text_color, font);
        
        pos += c.width + 1;
    }
}

void list::draw_items(graphic &gr_)
{
    if (!draw_callback)
    {
        return;
    }

    auto visible_item_count = get_visible_item_count();
    if (visible_item_count > item_count)
    {
        visible_item_count = item_count;
    }

    if (visible_item_count > item_count - start_item)
    {
        start_item = item_count - visible_item_count;
    }

    auto control_position = position();
    int32_t top_ = control_position.top + theme_dimension(tc, tv_border_width, theme_),
        left = control_position.left + theme_dimension(tc, tv_border_width, theme_),
        right = control_position.right - theme_dimension(tc, tv_border_width, theme_);

    for (auto i = 0; i != visible_item_count; ++i)
    {
        int32_t scrollbar_width = 0;
        if (mouse_on_control)
        {
            scrollbar_width = tiny_scrollbar_width;

            if (mouse_on_scrollbar)
            {
                scrollbar_width = full_scrollbar_width;
            }
        }

        auto top = (i * item_height) + top_;
        rect item_rect = { left, title_height + top, right - scrollbar_width, title_height + top + item_height };
        int32_t item = start_item + i;
        draw_callback(gr_, item, item_rect, item == selected_item_, columns);
    }
}

void list::draw_scrollbar(graphic &gr)
{
    rect bar_rect = { 0 }, top_button_rect = { 0 }, bottom_button_rect = { 0 }, slider_rect = { 0 };
    calc_scrollbar_params(&bar_rect, &top_button_rect, &bottom_button_rect, &slider_rect);

    if (slider_rect.bottom == 0)
    {
        return;
    }

    gr.draw_rect(bar_rect, theme_color(tc, tv_scrollbar, theme_));

    gr.draw_rect(top_button_rect, theme_color(tc, tv_scrollbar_slider, theme_));

    //DrawArrowUp(dc, { topButtonRect.right - 11, 7 });

    gr.draw_rect(bottom_button_rect, theme_color(tc, tv_scrollbar_slider, theme_));
    //DrawArrowDown(dc, { bottomButtonRect.right - 11, bottomButtonRect.bottom - 9 });

    gr.draw_rect(slider_rect, theme_color(tc, mouse_on_scrollbar ? tv_scrollbar_slider_acive : tv_scrollbar_slider, theme_));
}

int32_t list::get_visible_item_count() const
{
    if (item_height == 0 || title_height == -1)
    {
        return 0;
    }
    return static_cast<int32_t>(ceil(static_cast<double>(position_.height() - title_height) / item_height));
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

void list::calc_scrollbar_params(rect *bar_rect, rect *top_button_rect, rect *bottom_button_rect, rect *slider_rect, double *item_on_scroll_height)
{
    int32_t scrollbar_width = 0;
    if (mouse_on_control)
    {
        scrollbar_width = tiny_scrollbar_width;
    }
    else
    {
        return;
    }
    if (mouse_on_scrollbar)
    {
        scrollbar_width = full_scrollbar_width;
    }

    auto border_width = theme_dimension(tc, tv_border_width, theme_);

    const int32_t SB_WIDTH = scrollbar_width,
        SB_HEIGHT = full_scrollbar_width, SB_INDENT = 0, SB_SILDER_MIN_WIDTH = 5,
        SB_BUTTON_WIDTH = SB_WIDTH - SB_INDENT, SB_BUTTON_HEIGHT = SB_HEIGHT - SB_INDENT;

    auto control_pos = position();

    double client_height = control_pos.height() - (SB_HEIGHT * 2);
    auto visible_item_count = get_visible_item_count();
    if (visible_item_count >= item_count)
    {
        return;
    }

    double item_on_scroll_height_ = client_height / item_count;
    if (item_on_scroll_height)
    {
        *item_on_scroll_height = item_on_scroll_height_;
    }

    if (bar_rect)
    {
        *bar_rect = { control_pos.right - SB_WIDTH - border_width, control_pos.top + border_width, control_pos.right - border_width, control_pos.bottom - border_width };
    }

    if (top_button_rect && mouse_on_scrollbar)
    {
        *top_button_rect = { control_pos.right - SB_BUTTON_WIDTH - border_width, control_pos.top + SB_INDENT + border_width, control_pos.right - SB_INDENT - border_width, control_pos.top + SB_BUTTON_HEIGHT + border_width };
    }

    if (bottom_button_rect && mouse_on_scrollbar)
    {
        *bottom_button_rect = { control_pos.right - SB_BUTTON_WIDTH - border_width, control_pos.bottom - SB_BUTTON_HEIGHT - border_width, control_pos.right - SB_INDENT - border_width, control_pos.bottom - SB_INDENT - border_width };
    }

    if (slider_rect)
    {
        auto slider_top = control_pos.top + static_cast<int32_t>(round(item_on_scroll_height_ * start_item)) + border_width;
        auto slider_height = static_cast<int32_t>(floor(item_on_scroll_height_ * visible_item_count));

        if (slider_height < SB_SILDER_MIN_WIDTH)
        {
            slider_height = SB_SILDER_MIN_WIDTH;
        }

        *slider_rect = { control_pos.right - SB_BUTTON_WIDTH - border_width,
            SB_HEIGHT + slider_top,
            control_pos.right - SB_INDENT - border_width,
            SB_HEIGHT + slider_top + slider_height };

        if (mouse_on_scrollbar && slider_rect->bottom > bottom_button_rect->top)
        {
            slider_rect->move(0, bottom_button_rect->top - slider_rect->bottom);
        }
    }
}

}
