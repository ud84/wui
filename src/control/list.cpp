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

list::list(std::string_view theme_control_name_, std::shared_ptr<i_theme> theme__)
    : tcn(theme_control_name_),
    theme_(theme__),
    position_(),
    parent_(),
    my_control_sid(),
    showed_(true), enabled_(true), focused_(false), mouse_on_control(false), mouse_on_slider(false),
    columns_(),
    mode(list_mode::simple),
    item_count(0), selected_item_(0), active_item_(-1),
    title_height(-1),
    scroll_area(0),
    vert_scroll(new scroll(0, 0, orientation::vertical, std::bind(&list::on_scroll, this, std::placeholders::_1, std::placeholders::_2), scroll::tc, theme__)),
    draw_callback(),
    item_height_callback(),
    item_change_callback(),
    item_click_callback(),
    column_click_callback(),
    item_activate_callback(),
    scroll_callback()
{
}

list::~list()
{
    auto parent__ = parent_.lock();
    if (parent__)
    {
        parent__->remove_control(shared_from_this());
    }
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
    system_context ctx = { 0 };
    auto parent__ = parent_.lock();
    if (parent__)
    {
#ifdef _WIN32
        ctx = parent__->context();
#elif __linux__
        ctx = { parent__->context().display, parent__->context().connection, parent__->context().screen, gr.drawable() };
#endif
    }

    graphic mem_gr(ctx);
    mem_gr.init({ 0, 0, position_.width() - border_width * 2, position_.height() - border_width * 2 }, theme_color(tcn, tv_background, theme_));

    calc_title_height(mem_gr);

    draw_items(mem_gr);

    draw_titles(mem_gr);

    gr.draw_graphic({control_pos.left + border_width,
            control_pos.top + border_width,
            control_pos.width() - border_width,
            control_pos.height() - border_width },
        mem_gr, 0, 0);

    if ((mouse_on_control || focused_) && has_scrollbar())
    {
        vert_scroll->draw(gr, {});
    }
}

void list::receive_control_events(const event &ev)
{
    if (!showed_ || !enabled_)
    {
        return;
    }

    if (ev.type == event_type::mouse)
    {
        if (has_scrollbar())
        {
            if (vert_scroll->position().in(ev.mouse_event_.x, ev.mouse_event_.y))
            {
                if (!mouse_on_slider)
                {
                    mouse_on_slider = true;

                    event sev = ev;
                    sev.mouse_event_.type = wui::mouse_event_type::enter;

                    return vert_scroll->receive_control_events(sev);
                }

                return vert_scroll->receive_control_events(ev);
            }
            else
            {
                if (mouse_on_slider)
                {
                    mouse_on_slider = false;

                    event sev = ev;
                    sev.mouse_event_.type = wui::mouse_event_type::leave;

                    return vert_scroll->receive_control_events(sev);
                }
            }
        }

        switch (ev.mouse_event_.type)
        {
            case mouse_event_type::enter:
                if (has_scrollbar())
                {
                    vert_scroll->show();
                }
                mouse_on_control = true;
            break;
            case mouse_event_type::leave:
                mouse_on_control = false;
                mouse_on_slider = false;
                redraw();
            break;
            case mouse_event_type::left_down:
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
                        item_click_callback(click_button::left, selected_item_, ev.mouse_event_.x, ev.mouse_event_.y);
                    }
                }
            break;
            case mouse_event_type::right_up:
                if (ev.mouse_event_.y - position().top <= title_height)
                {
                    return;
                }

                update_selected_item(ev.mouse_event_.y);

                if (item_click_callback)
                {
                    item_click_callback(click_button::right, selected_item_, ev.mouse_event_.x, ev.mouse_event_.y);
                }
            break;
            case mouse_event_type::left_double:
                if (ev.mouse_event_.y - position().top <= title_height)
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
                if (mode == list_mode::simple || mode == list_mode::simple_topmost)
                {
                    update_active_item(ev.mouse_event_.y);
                }
                else if (mode == list_mode::auto_select)
                {
                    update_selected_item(ev.mouse_event_.y);
                }
            }
            break;
            case mouse_event_type::wheel:
                if (mode == list_mode::simple || mode == list_mode::simple_topmost)
                {
                    update_active_item(ev.mouse_event_.y);
                }
                else if (mode == list_mode::auto_select)
                {
                    update_selected_item(ev.mouse_event_.y);
                }

                if (ev.mouse_event_.wheel_delta > 0)
                {
                    vert_scroll->scroll_up();
                }
                else
                {
                    vert_scroll->scroll_down();
                }
            break;
            default: break;
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
                    case vk_home:
                    {
                        if (selected_item_ != 0)
                        {
                            vert_scroll->set_scroll_pos(0);
                            selected_item_ = 0;
                            redraw();
                        }
                    }
                    break;
                    case vk_end:
                    {
                        if (selected_item_ != item_count - 1)
                        {
                            vert_scroll->set_scroll_pos(scroll_area - position_.height());

                            selected_item_ = static_cast<int32_t>(item_count - 1);
                            redraw();
                        }
                    }
                    break;
                    case vk_up:
                        if (selected_item_ != 0)
                        {
                            --selected_item_;
                            
                            auto selected_item_top = get_item_top(selected_item_);
                            if (selected_item_top < vert_scroll->get_scroll_pos())
                            {
                                vert_scroll->set_scroll_pos(selected_item_top);
                            }

                            redraw();
                            
                            if (item_change_callback)
                            {
                                item_change_callback(selected_item_);
                            }
                        }
                    break;
                    case vk_down:
                        if (selected_item_ != item_count - 1)
                        {
                            ++selected_item_;
                            
                            auto selected_item_bottom = get_item_top(selected_item_) + get_item_height(selected_item_);
                            if (selected_item_bottom > position_.height())
                            {
                                vert_scroll->set_scroll_pos(selected_item_bottom - position_.height() + title_height);
                            }
                            
                            redraw();

                            if (item_change_callback)
                            {
                                item_change_callback(selected_item_);
                            }
                        }
                    break;
                    case vk_page_up:
                        if (selected_item_ != 0)
                        {
                            const int32_t diff_scroll = 10;

                            if (selected_item_ > diff_scroll)
                            {
                                selected_item_ -= diff_scroll;
                            }
                            else
                            {
                                selected_item_ = 0;
                            }

                            vert_scroll->set_scroll_pos(get_item_top(selected_item_));

                            redraw();

                            if (item_change_callback)
                            {
                                item_change_callback(selected_item_);
                            }
                        }
                    break;
                    case vk_page_down:
                        if (selected_item_ != item_count - 1 && item_count != 0)
                        {
                            int32_t diff_scroll = 10;
                            if (diff_scroll > item_count)
                            {
                                diff_scroll = item_count;
                            }

                            if (selected_item_ < item_count - diff_scroll)
                            {
                                selected_item_ += diff_scroll;
                            }
                            else
                            {
                                selected_item_ = static_cast<int32_t>(item_count - 1);
                            }

                            vert_scroll->set_scroll_pos(get_item_top(selected_item_) + get_item_height(selected_item_) - position_.height() + title_height);
                                                        
                            redraw();

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
                    //end_work();
                }
            break;
            default: break;
        }
    }
    else if (ev.type == event_type::internal)
    {
        switch (ev.internal_event_.type)
        {
            case internal_event_type::set_focus:
                focused_ = true;

                if (has_scrollbar())
                {
                    vert_scroll->show();
                }

                redraw();
            break;
            case internal_event_type::remove_focus:
                focused_ = false;

                vert_scroll->hide();

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

void list::set_position(const rect &position__, bool redraw)
{
    update_control_position(position_, position__, showed_ && redraw, parent_);

    auto border_width = theme_dimension(tcn, tv_border_width, theme_);

    vert_scroll->set_position({ position_.right - 14 - border_width,
        position_.top + border_width,
        position_.right - border_width,
        position_.bottom - border_width });

    update_scroll_area();
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

    window->add_control(vert_scroll, { 0 });
}

std::weak_ptr<window> list::parent() const
{
    return parent_;
}

void list::clear_parent()
{
    auto parent__ = parent_.lock();
    if (parent__)
    {
        parent__->remove_control(vert_scroll);

        parent__->unsubscribe(my_control_sid);
        my_control_sid.clear();
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

error list::get_error() const
{
    return {};
}

void list::update_theme_control_name(std::string_view theme_control_name)
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
    selected_item_ = n_item;

    make_selected_visible();

    redraw();

    if (item_change_callback)
    {
        item_change_callback(selected_item_);
    }
}

int32_t list::selected_item() const
{
    return selected_item_;
}

void list::set_column_width(int32_t n_column, int32_t width)
{
    if (static_cast<int32_t>(columns_.size()) > n_column)
    {
        columns_[n_column].width = width;
        redraw();
    }
}

int32_t list::get_item_height(int32_t n_item) const
{
    int32_t height = -1;
    if (item_height_callback)
    {
        item_height_callback(n_item, height);
    }
    return height;
}

void list::set_item_count(int32_t count)
{
    if (item_count > count)
    {
        vert_scroll->set_scroll_pos(0);
    }

    item_count = count;

    update_scroll_area();

    redraw();
}

int32_t list::get_item_count() const
{
    return item_count;
}

void list::scroll_to_start()
{
    vert_scroll->set_scroll_pos(0);
    redraw();
}

void list::scroll_to_end()
{
    vert_scroll->set_scroll_pos(scroll_area);
    redraw();
}

int32_t list::get_item_top(int32_t n_item) const
{
    if (n_item < 0)
    {
        return 0;
    }

    int32_t top = 0;

    for (int32_t i = 0; i != n_item; ++i)
    {
        auto height = get_item_height(i);
        top += height != -1 ? height : 0;
    }

    return top;
}

void list::set_draw_callback(std::function<void(graphic&, int32_t, const rect&, item_state state)> draw_callback_)
{
    draw_callback = draw_callback_;
}

void list::set_item_height_callback(std::function<void(int32_t, int32_t&)> item_height_callback_)
{
    item_height_callback = item_height_callback_;
}

void list::set_item_click_callback(std::function<void(click_button, int32_t, int32_t, int32_t)> item_click_callback_)
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

void list::set_scroll_callback(std::function<void(scroll_state, int32_t)> scroll_callback_)
{
    scroll_callback = scroll_callback_;
}

void list::on_scroll(scroll_state ss, int32_t v)
{
    redraw();

    if (scroll_callback)
    {
        scroll_callback(ss, v);
    }
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

        auto scroll_pos = vert_scroll->get_scroll_pos();

        auto top = control_pos.top + get_item_top(item) + title_height - scroll_pos;

        auto parent__ = parent_.lock();
        if (parent__)
        {
            auto height = get_item_height(item);
            if (height != -1)
            {
                parent__->redraw({ control_pos.left, top, control_pos.right, top + height + 1 });
            }
        }
    }
}

void list::calc_title_height(graphic &gr_)
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
}

void list::draw_titles(graphic &gr_)
{
    auto font = theme_font(tcn, tv_font, theme_);
    auto text_indent = 5;

    auto border_width = theme_dimension(tcn, tv_border_width, theme_);
    auto title_color = theme_color(tcn, tv_title, theme_);
    auto title_text_color = theme_color(tcn, tv_title_text, theme_);
    
    int32_t left = 0;

    gr_.draw_rect({ left, 0, position_.width() - 1, title_height }, title_color);

    for (auto &c : columns_)
    {
        gr_.draw_rect({ left, 0, left + c.width - 1, title_height }, title_color);
        gr_.draw_text({ left + text_indent, text_indent, 0, 0 }, c.caption, title_text_color, font);
        
        left += c.width + 1;
    }
}

void list::draw_items(graphic &gr_)
{
    if (!draw_callback || position_.height() == 0)
    {
        return;
    }

    int32_t first_item = -1, item_bottom = 0;
    auto scroll_pos = vert_scroll->get_scroll_pos();
    while (scroll_pos >= item_bottom && first_item < item_count)
    {
        ++first_item;
        item_bottom = get_item_top(first_item) + get_item_height(first_item);
    }

    int32_t last_item = -1, item_top = 0;
    while (position_.height() > item_top && last_item < item_count)
    {
        ++last_item;
        item_top = get_item_top(last_item) - scroll_pos;
    }

    if (last_item < first_item || last_item == first_item)
    {
        return;
    }

    if (last_item > item_count)
    {
        last_item = item_count;
    }

    auto border_width = theme_dimension(tcn, tv_border_width, theme_);

    int32_t top_ = border_width + title_height - scroll_pos,
        left = border_width,
        right = position_.width() - border_width;

    for (auto item = first_item; item != last_item; ++item)
    {
        auto item_height = get_item_height(item);
        auto top = get_item_top(item) + top_;

        rect item_rect = { left, top, right, top + item_height - border_width };

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
    return scroll_area + position_.height() > position_.height();
}

void list::update_selected_item(int32_t y)
{
    auto border_width = theme_dimension(tcn, tv_border_width, theme_);

    auto scroll_pos = vert_scroll->get_scroll_pos();

    auto pos = (y - position().top - title_height - border_width) + scroll_pos;

    int32_t item = -1, item_start = 0, item_end = 0;
    while (item != item_count)
    {
        if (item_start <= pos && item_end > pos)
        {
            break;
        }
        else
        {
            ++item;
        }
        item_start = get_item_top(item);
        
        auto height = get_item_height(item);
        item_end = height != -1 ? item_start + height : 0;
    }

    if (item != selected_item_)
    {
        int32_t old_selected = selected_item_;

        selected_item_ = item < item_count ? item : -1;

        redraw_item(old_selected);
        redraw_item(selected_item_);

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

    auto scroll_pos = vert_scroll->get_scroll_pos();

    auto pos = (y - position().top - title_height - border_width) + scroll_pos;

    active_item_ = -1;

    int32_t item_start = 0, item_end = 0;
    while (active_item_ != item_count)
    {
        if (item_start <= pos && item_end > pos)
        {
            break;
        }
        else
        {
            ++active_item_;
        }
        item_start = get_item_top(active_item_);

        auto height = get_item_height(active_item_);
        item_end = height != -1 ? item_start + height : 0;
    }
   
    if (prev_active_item_ != active_item_)
    {
        redraw();
    }
}

void list::make_selected_visible()
{
    auto selected_item_top = get_item_top(selected_item_);
    auto selected_item_bottom = selected_item_top + get_item_height(selected_item_);

    auto scroll_pos = vert_scroll->get_scroll_pos();
    if (selected_item_top < scroll_pos)
    {
        scroll_pos = selected_item_top;
        return;
    }

    if (position_.height() <= 0 || position_.height() > selected_item_bottom || position_.height() - scroll_pos >= selected_item_top)
    {
        return;
    }

    if (selected_item_bottom > scroll_pos)
    {
        scroll_pos = selected_item_top;
    }

    if (selected_item_bottom - scroll_pos < position_.height() - title_height)
    {
        scroll_pos = selected_item_bottom - position_.height() + title_height;
    }
}

void list::update_scroll_area()
{
    bool scrolled_down = false;
    if (scroll_area != 0 && vert_scroll->get_scroll_pos() == scroll_area)
    {
        scrolled_down = true;
    }

    scroll_area = title_height + get_item_top(item_count - 1) + get_item_height(item_count - 1) - position_.height();

    vert_scroll->set_area(scroll_area);

    if (vert_scroll->get_scroll_pos() > scroll_area)
    {
        vert_scroll->set_scroll_pos(scroll_area);
    }

    if (scrolled_down)
    {
        vert_scroll->set_scroll_pos(scroll_area);
    }
}

}
