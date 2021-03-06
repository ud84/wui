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
#include <cmath>

namespace wui
{

list::list(const std::string &theme_control_name_, std::shared_ptr<i_theme> theme__)
    : tcn(theme_control_name_),
    theme_(theme__),
    position_(),
    parent_(),
    my_control_sid(), my_plain_sid(),
    showed_(true), enabled_(true), focused_(false), mouse_on_control(false),
    columns_(),
    mode(list_mode::simple),
    item_height(28), item_count(0), selected_item_(0), active_item_(-1), start_item(0),
    worker_action_(worker_action::undefined),
    worker(),
    progress(0),
    scrollbar_state_(scrollbar_state::hide),
    slider_scrolling(false),
    prev_scroll_pos(0),
    title_height(-1),
    draw_callback(),
    item_change_callback(),
    item_click_callback(),
    column_click_callback(),
    item_activate_callback(),
    item_right_click_callback()
{
}

list::~list()
{
    auto parent__ = parent_.lock();
    if (parent__)
    {
        parent__->remove_control(shared_from_this());
    }

    end_work();
}

void list::draw(graphic &gr, const rect &)
{
    if (!showed_)
    {
        return;
    }

    auto control_pos = position();

    auto border_width = theme_dimension(tcn, tv_border_width, theme_);

    gr.draw_rect(control_pos,
        !focused_ ? theme_color(tcn, tv_border, theme_) : theme_color(tcn, tv_focused_border, theme_),
        theme_color(tcn, tv_background, theme_),
        border_width,
        theme_dimension(tcn, tv_round, theme_));

    /// Create memory dc for inner content   
#ifdef _WIN32
    system_context ctx = { 0, gr.drawable() };
#elif __linux__
    system_context ctx = { 0 };
    auto parent__ = parent_.lock();
    if (parent__)
    {
        ctx = { parent__->context().display, parent__->context().connection, parent__->context().screen, gr.drawable() };
    }
#endif
    graphic mem_gr(ctx);
    mem_gr.init({ 0, 0, position_.width() - border_width * 2, position_.height() - border_width * 2 }, theme_color(tcn, tv_background, theme_));

    draw_titles(mem_gr);

    draw_items(mem_gr);

    draw_scrollbar(mem_gr);

    gr.draw_graphic({ control_pos.left + border_width,
            control_pos.top + border_width,
            control_pos.width() - border_width,
            control_pos.height() - border_width },
        mem_gr, 0, 0);
}

void list::receive_control_events(const event &ev)
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
                    auto parent__ = parent_.lock();
                    if (parent__)
                    {
                        set_cursor(parent__->context(), cursor::default_);
                    }
                    if (has_scrollbar())
                    {
                        scrollbar_state_ = scrollbar_state::tiny;
                        redraw();
                    }
                }
            }
            break;
            case mouse_event_type::leave:
                mouse_on_control = false;
                if (!slider_scrolling)
                {
                    scrollbar_state_ = scrollbar_state::hide;
                    active_item_ = -1;
                    redraw();
                }
            break;
            case mouse_event_type::left_down:
                if (is_click_on_scrollbar(ev.mouse_event_.x))
                {
                    active_item_ = -1;

                    rect bar_rect = { 0 }, top_button_rect = { 0 }, bottom_button_rect = { 0 }, slider_rect = { 0 };
                    calc_scrollbar_params(false, &bar_rect, &top_button_rect, &bottom_button_rect, &slider_rect);

                    if (ev.mouse_event_.y <= top_button_rect.bottom)
                    {
                        start_work(worker_action::scroll_up);
                    }
                    else if (ev.mouse_event_.y >= bottom_button_rect.top)
                    {
                        start_work(worker_action::scroll_down);
                    }
                    else if (ev.mouse_event_.y < slider_rect.top)
                    {
                        while (ev.mouse_event_.y < slider_rect.top)
                        {
                            scroll_up();
                            calc_scrollbar_params(false, &bar_rect, &top_button_rect, &bottom_button_rect, &slider_rect);
                        }
                    }
                    else if (ev.mouse_event_.y > slider_rect.bottom)
                    {
                        while (ev.mouse_event_.y > slider_rect.bottom)
                        {
                            scroll_down();
                            calc_scrollbar_params(false, &bar_rect, &top_button_rect, &bottom_button_rect, &slider_rect);
                        }
                    }
                    else if (ev.mouse_event_.y >= slider_rect.top && ev.mouse_event_.y <= slider_rect.bottom)
                    {
                        slider_scrolling = true;
                        prev_scroll_pos = ev.mouse_event_.y;
                    }
                }
                else
                {
                    if (ev.mouse_event_.y - position().top <= title_height)
                    {
                        int32_t pos = 0, n = 0;
                        for (auto &c : columns_)
                        {
                            if (ev.mouse_event_.x >= pos && ev.mouse_event_.x < pos + c.width)
                            {
                                if (column_click_callback)
                                {
                                    column_click_callback(n);
                                }
                                return;
                            }

                            pos += c.width;
                            ++n;
                        }
                        return;
                    }
                    else
                    {
                        update_selected_item(ev.mouse_event_.y);

                        if (item_click_callback)
                        {
                            item_click_callback(selected_item_, ev.mouse_event_.x - position().left);
                        }
                    }
                }
            break;
            case mouse_event_type::left_up:
            {
                slider_scrolling = false;
                end_work();
            }
            break;
            case mouse_event_type::right_up:
                if (ev.mouse_event_.y - position().top <= title_height || is_click_on_scrollbar(ev.mouse_event_.x))
                {
                    return;
                }

                update_selected_item(ev.mouse_event_.y);

                if (item_right_click_callback)
                {
                    item_right_click_callback(selected_item_, ev.mouse_event_.x, ev.mouse_event_.y);
                }
            break;
            case mouse_event_type::left_double:
                if (ev.mouse_event_.y - position().top <= title_height || is_click_on_scrollbar(ev.mouse_event_.x))
                {
                    return;
                }

                update_selected_item(ev.mouse_event_.y);
                
                if (selected_item_ != -1 && item_activate_callback)
                {
                    item_activate_callback(selected_item_);
                }
            break;
            case mouse_event_type::move:
            {
                if (slider_scrolling)
                {
                    return move_slider(ev.mouse_event_.y);
                }

                auto has_scrollbar_ = has_scrollbar();
                if (has_scrollbar_ && ev.mouse_event_.x > position().right - full_scrollbar_width - theme_dimension(tcn, tv_border_width, theme_) * 2)
                {
                    if (scrollbar_state_ != scrollbar_state::full)
                    {
                        scrollbar_state_ = scrollbar_state::full;
                        progress = 0;
                        start_work(worker_action::scrollbar_show);
                    }
                }
                else
                {
                    if (mode == list_mode::simple || mode == list_mode::simple_topmost)
                    {
                        update_active_item(ev.mouse_event_.y);
                    }
                    else if (mode == list_mode::auto_select)
                    {
                        update_selected_item(ev.mouse_event_.y);
                    }

                    if (has_scrollbar_ && scrollbar_state_ == scrollbar_state::full)
                    {
                        scrollbar_state_ = scrollbar_state::tiny;
                        redraw();
                    }
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

                if (mode == list_mode::simple || mode == list_mode::simple_topmost)
                {
                    update_active_item(ev.mouse_event_.y);
                }
                else if (mode == list_mode::auto_select)
                {
                    update_selected_item(ev.mouse_event_.y);
                }
            break;
        }
    }
    else if (ev.type == event_type::keyboard)
    {
        switch (ev.keyboard_event_.type)
        {
            case keyboard_event_type::down:
                active_item_ = -1;
                switch (ev.keyboard_event_.key[0])
                {
                    case vk_home: case vk_page_up:
                    {
                        while (selected_item_ != 0)
                        {
                            --selected_item_;

                            if (selected_item_ < start_item + 1)
                            {
                                scroll_up();
                            }
                        }
                        redraw();
                    }
                    break;
                    case vk_end: case vk_page_down:
                    {
                        auto visible_item_count = get_visible_item_count();

                        while (selected_item_ != item_count - 1)
                        {
                            ++selected_item_;

                            if (selected_item_ > visible_item_count + start_item - 1)
                            {
                                scroll_down();
                            }
                        }
                        redraw();
                    }
                    break;
                    case vk_up:
                        if (selected_item_ != 0)
                        {
                            --selected_item_;

                            if (selected_item_ != 0 && selected_item_ < start_item + 1)
                            {
                                scroll_up();
                            }
                            else
                            {
                                redraw();
                            }

                            if (item_change_callback)
                            {
                                item_change_callback(selected_item_);
                            }
                        }
                    break;
                    case vk_down:
                        if (selected_item_ != item_count)
                        {
                            ++selected_item_;

                            auto visible_item_count = get_visible_item_count();

                            if (selected_item_ > visible_item_count + start_item - 1)
                            {
                                scroll_down();
                            }
                            else
                            {
                                redraw();
                            }

                            if (item_change_callback)
                            {
                                item_change_callback(selected_item_);
                            }
                        }
                    break;
                }
            break;
            case keyboard_event_type::up:
                if (ev.keyboard_event_.key[0] == vk_up || ev.keyboard_event_.key[0] == vk_down)
                {
                    end_work();
                }
            break;
        }
    }
    else if (ev.type == event_type::internal)
    {
        switch (ev.internal_event_.type)
        {
            case internal_event_type::set_focus:
                if (enabled_ && showed_)
                {
                    focused_ = true;

                    scrollbar_state_ = scrollbar_state::tiny;

                    redraw();
                }
            break;
            case internal_event_type::remove_focus:
                focused_ = false;

                scrollbar_state_ = scrollbar_state::hide;

                redraw();
            break;
            case internal_event_type::execute_focused:
                if (selected_item_ != -1 && item_activate_callback)
                {
                    item_activate_callback(selected_item_);
                }
            break;
        }
    }
}

void list::receive_plain_events(const event &ev)
{
    if (!mouse_on_control && ev.type == event_type::mouse)
    {
        switch (ev.mouse_event_.type)
        {
            case mouse_event_type::move:
                if (slider_scrolling)
                {
                    move_slider(ev.mouse_event_.y);
                }
            break;
            case mouse_event_type::left_up:
                end_work();

                slider_scrolling = false;
                scrollbar_state_ = scrollbar_state::hide;
                active_item_ = -1;
                
                redraw();
            break;
        }
    }
}

void list::set_position(const rect &position__, bool redraw)
{
    update_control_position(position_, position__, showed_ && redraw, parent_);
}

rect list::position() const
{
    return get_control_position(position_, parent_);
}

void list::set_parent(std::shared_ptr<window> window)
{
    parent_ = window;
    my_control_sid = window->subscribe(std::bind(&list::receive_control_events, this, std::placeholders::_1),
        static_cast<event_type>(static_cast<uint32_t>(event_type::internal) | static_cast<uint32_t>(event_type::mouse) | static_cast<uint32_t>(event_type::keyboard)),
        shared_from_this());

    my_plain_sid = window->subscribe(std::bind(&list::receive_plain_events, this, std::placeholders::_1), event_type::mouse);
}

std::weak_ptr<window> list::parent() const
{
    return parent_;
}

void list::clear_parent()
{
    end_work();

    auto parent__ = parent_.lock();
    if (parent__)
    {
        parent__->unsubscribe(my_control_sid);
        my_control_sid.clear();
        
        parent__->unsubscribe(my_plain_sid);
        my_plain_sid.clear();
    }
    parent_.reset();
}

void list::set_topmost(bool yes)
{
    mode = yes ? list_mode::simple_topmost : list_mode::simple;
}

bool list::topmost() const
{
    return mode == list_mode::auto_select || mode == list_mode::simple_topmost;
}

bool list::focused() const
{
    return enabled_ && showed_ && focused_;
}

bool list::focusing() const
{
    return enabled_ && showed_;
}

void list::update_theme_control_name(const std::string &theme_control_name)
{
    tcn = theme_control_name;
    update_theme(theme_);
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
        auto parent__ = parent_.lock();
        if (parent__)
        {
            parent__->redraw(position(), true);
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

void list::update_columns(const std::vector<column> &columns__)
{
    columns_ = columns__;
    title_height = -1;
    redraw();
}

const std::vector<list::column> &list::columns()
{
    return columns_;
}

void list::set_mode(list_mode mode_)
{
    mode = mode_;
}

void list::select_item(int32_t n_item)
{
    int32_t prev_selected_item_ = selected_item_;
    
    selected_item_ = n_item;

    if (item_change_callback)
    {
        item_change_callback(selected_item_);
    }

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
    if (static_cast<int32_t>(columns_.size()) > n_item)
    {
        columns_[n_item].width = width;
        redraw();
    }
}

void list::set_item_height(int32_t height)
{
    item_height = height;
}

int32_t list::get_item_height() const
{
    return item_height;
}

void list::set_item_count(int32_t count)
{
    item_count = count;
    redraw();
}

void list::set_draw_callback(std::function<void(graphic&, int32_t, const rect&, item_state state)> draw_callback_)
{
    draw_callback = draw_callback_;
}

void list::set_item_click_callback(std::function<void(int32_t, int32_t)> item_click_callback_)
{
    item_click_callback = item_click_callback_;
}

void list::set_item_change_callback(std::function<void(int32_t)> item_change_callback_)
{
    item_change_callback = item_change_callback_;
}

void list::set_item_activate_callback(std::function<void(int32_t)> item_activate_callback_)
{
    item_activate_callback = item_activate_callback_;
}

void list::set_column_click_callback(std::function<void(int32_t)> column_click_callback_)
{
    column_click_callback = column_click_callback_;
}

void list::set_item_right_click_callback(std::function<void(int32_t, int32_t, int32_t)> item_right_click_callback_)
{
    item_right_click_callback = item_right_click_callback_;
}

void list::redraw()
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

void list::redraw_item(int32_t item)
{
    if (showed_)
    {
        auto control_pos = position();

        auto top = control_pos.top + (item_height * item) + title_height;

        auto parent__ = parent_.lock();
        if (parent__)
        {
            parent__->redraw({ control_pos.left, top, control_pos.right, top + item_height + 1 });
        }
    }
}

void list::draw_titles(graphic &gr_)
{
    auto font = theme_font(tcn, tv_font, theme_);
    auto text_indent = 5;

    if (title_height == -1)
    {
        if (columns_.empty())
        {
            title_height = 0;
            return;
        }

        auto text_dimensions = gr_.measure_text("Qq", font);

        title_height = text_dimensions.width() + text_indent * 2;
    }

    auto border_width = theme_dimension(tcn, tv_border_width, theme_);
    auto title_color = theme_color(tcn, tv_title, theme_);
    auto title_text_color = theme_color(tcn, tv_title_text, theme_);
    
    int32_t left = 0;

    for (auto &c : columns_)
    {
        gr_.draw_rect({ left, 0, left + c.width - 1, title_height }, title_color);
        gr_.draw_text({ left + text_indent, text_indent, 0, 0 }, c.caption, title_text_color, font);
        
        left += c.width + 1;
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

    auto border_width = theme_dimension(tcn, tv_border_width, theme_);

    int32_t top_ = border_width,
        left = border_width,
        right = position_.width() - border_width;

    int32_t scrollbar_width = 0;
    if (has_scrollbar())
    {
        if (scrollbar_state_ == scrollbar_state::tiny)
        {
            scrollbar_width = tiny_scrollbar_width;
        }
        else if (scrollbar_state_ == scrollbar_state::full)
        {
            scrollbar_width = full_scrollbar_width;
        }
    }

    for (auto i = 0; i != visible_item_count; ++i)
    {
        auto top = (i * item_height) + top_;
        rect item_rect = { left, title_height + top, right - scrollbar_width, title_height + top + item_height - border_width };
        int32_t item = start_item + i;

        item_state state = item_state::normal;

        if (item == active_item_)
        {
            state = item_state::active;
        }
        if (item == selected_item_)
        {
            state = item_state::selected;
        }

        draw_callback(gr_, item, item_rect, state);
    }
}

bool list::has_scrollbar()
{
    return get_visible_item_count() < item_count;
}

void list::draw_scrollbar(graphic &gr)
{
    rect bar_rect = { 0 }, top_button_rect = { 0 }, bottom_button_rect = { 0 }, slider_rect = { 0 };
    calc_scrollbar_params(true, &bar_rect, &top_button_rect, &bottom_button_rect, &slider_rect);

    if (slider_rect.bottom == 0)
    {
        return;
    }

    gr.draw_rect(bar_rect, theme_color(tcn, tv_scrollbar, theme_));

    gr.draw_rect(top_button_rect, theme_color(tcn, tv_scrollbar_slider, theme_));
    if (scrollbar_state_ == scrollbar_state::full)
    {
        draw_arrow_up(gr, top_button_rect);
    }

    gr.draw_rect(bottom_button_rect, theme_color(tcn, tv_scrollbar_slider, theme_));
    if (scrollbar_state_ == scrollbar_state::full)
    {
        draw_arrow_down(gr, bottom_button_rect);
    }

    gr.draw_rect(slider_rect, theme_color(tcn, scrollbar_state_ == scrollbar_state::full ? tv_scrollbar_slider_acive : tv_scrollbar_slider, theme_));
}

void list::draw_arrow_up(graphic &gr, rect button_pos)
{
    auto color = theme_color(tcn, tv_scrollbar_slider_acive, theme_);

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

void list::draw_arrow_down(graphic &gr, rect button_pos)
{
    auto color = theme_color(tcn, tv_scrollbar_slider_acive, theme_);

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

int32_t list::get_visible_item_count() const
{
    if (item_height == 0 || title_height == -1)
    {
        return 0;
    }
    return static_cast<int32_t>(floor(static_cast<double>(position_.height() - title_height) / item_height));
}

void list::move_slider(int32_t y)
{
    int32_t diff = prev_scroll_pos - y;
    double item_on_scroll_height = static_cast<double>(position_.height() - full_scrollbar_width * 2) / (item_count - get_visible_item_count());
    double diff_abs = labs(diff);

    if (item_on_scroll_height == 0.)
    {
        return;
    }

    auto count = static_cast<int32_t>(round(diff_abs / item_on_scroll_height));

    rect slider_rect = { 0 };
    calc_scrollbar_params(false, nullptr, nullptr, nullptr, &slider_rect);

    if (diff > 0)
    {
        for (auto i = 0; i != count; ++i)
        {
            scroll_up();
        }

        if (slider_rect.bottom < y)
        {
            prev_scroll_pos = y - (y - slider_rect.bottom);
            return;
        }
    }
    else
    {
        for (auto i = 0; i != count; ++i)
        {
            scroll_down();
        }

        if (slider_rect.top > y)
        {
            prev_scroll_pos = y + (slider_rect.top - y);
            return;
        }
    }

    if (count != 0)
    {
        prev_scroll_pos = y;
    }
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

void list::calc_scrollbar_params(bool drawing_coordinates, rect *bar_rect, rect *top_button_rect, rect *bottom_button_rect, rect *slider_rect)
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

    auto border_width = theme_dimension(tcn, tv_border_width, theme_);

    const int32_t SB_WIDTH = scrollbar_width,
        SB_HEIGHT = full_scrollbar_width, SB_SILDER_MIN_WIDTH = 5,
        SB_BUTTON_WIDTH = SB_WIDTH, SB_BUTTON_HEIGHT = SB_HEIGHT;

    auto control_pos = position();
    if (drawing_coordinates)
    {
        control_pos = { 0, 0, position_.width(), position_.height() };
    }

    double client_height = control_pos.height() - (SB_HEIGHT * 2);
    auto visible_item_count = get_visible_item_count();
    if (visible_item_count >= item_count)
    {
        return;
    }

    double item_on_scroll_height_ = client_height / item_count;

    if (bar_rect)
    {
        *bar_rect = { control_pos.right - SB_WIDTH - border_width, control_pos.top + border_width, control_pos.right - border_width, control_pos.bottom - border_width };
    }

    if (top_button_rect && scrollbar_state_ == scrollbar_state::full)
    {
        *top_button_rect = { control_pos.right - SB_BUTTON_WIDTH - border_width, control_pos.top + border_width, control_pos.right - border_width, control_pos.top + SB_BUTTON_HEIGHT + border_width };
    }

    if (bottom_button_rect && scrollbar_state_ == scrollbar_state::full)
    {
        *bottom_button_rect = { control_pos.right - SB_BUTTON_WIDTH - border_width, control_pos.bottom - SB_BUTTON_HEIGHT - border_width, control_pos.right - border_width, control_pos.bottom - border_width };
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
            control_pos.right - border_width,
            SB_HEIGHT + slider_top + slider_height };

        if (scrollbar_state_ == scrollbar_state::full && bottom_button_rect && slider_rect->bottom > bottom_button_rect->top)
        {
            slider_rect->move(0, bottom_button_rect->top - slider_rect->bottom);
        }
    }
}

bool list::is_click_on_scrollbar(int32_t x)
{
    return x >= position().right - full_scrollbar_width;
}

void list::update_selected_item(int32_t y)
{
    auto border_width = theme_dimension(tcn, tv_border_width, theme_);
    auto item = static_cast<int32_t>(floor((double)(y - position().top - title_height - border_width) / item_height)) + start_item;

    if (item > item_count)
    {
        item = -1;
    }

    if (item != selected_item_)
    {
        int32_t old_selected = selected_item_;

        selected_item_ = item;

        redraw_item(item - start_item);
        redraw_item(old_selected - start_item);

        if (item_change_callback)
        {
            item_change_callback(selected_item_);
        }
    }
}

void list::update_active_item(int32_t y)
{
    int32_t prev_active_item_ = active_item_;
    
    auto border_width = theme_dimension(tcn, tv_border_width, theme_);
    active_item_ = static_cast<int32_t>(floor((double)(y - position().top - title_height - border_width) / item_height)) + start_item;

    if (active_item_ > item_count)
    {
        active_item_ = -1;
    }

    if (prev_active_item_ != active_item_)
    {
        redraw();
    }
}

void list::start_work(worker_action action)
{
    worker_action_ = action;
    
    if (!worker_runned)
    {
        worker_runned = true;
        if (worker.joinable()) worker.join();
        worker = std::thread(std::bind(&list::work, this));
    }
}

void list::work()
{
    while (worker_runned)
    {
        switch (worker_action_)
        {
        case worker_action::scroll_up:
            scroll_up();
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        break;
        case worker_action::scroll_down:
            scroll_down();
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        break;
        case worker_action::scrollbar_show:
            if (progress < full_scrollbar_width)
            {
                progress += 4;

                auto parent__ = parent_.lock();
                if (parent__)
                {
                    auto control_pos = position();
                    parent__->redraw({ control_pos.right - progress, control_pos.top, control_pos.right, control_pos.bottom });
                }
            }
            else
            {
                worker_runned = false;
            }

            std::this_thread::sleep_for(std::chrono::milliseconds(20));
        break;
        }
    }
}

void list::end_work()
{
    worker_runned = false;
    if (worker.joinable()) worker.join();
}

}
