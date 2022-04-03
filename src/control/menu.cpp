//
// Copyright (c) 2021-2022 Anton Golovkov (udattsk at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/ud84/wui
//

#include <wui/control/menu.hpp>

#include <wui/window/window.hpp>

#include <wui/control/image.hpp>
#include <wui/control/list.hpp>

#include <wui/theme/theme.hpp>

#include <wui/system/tools.hpp>

#include <algorithm>

namespace wui
{

int32_t calc_items_count(const menu_items_t &items)
{
    int32_t count = 0;
    for (auto &item : items)
    {
        ++count;
        if (!item.children.empty() && item.state == menu_item_state::expanded)
        {
            count += calc_items_count(item.children);
        }
    }
    return count;
}

menu_item *get_item(menu_items_t &items, int32_t n_item, int32_t pos = 0, int32_t level = 0)
{
    int32_t count = pos;
    for (auto &item : items)
    {
        if (count == n_item)
        {
            item.level = level;
            return &item;
        }

        if (!item.children.empty() && item.state == menu_item_state::expanded)
        {
            auto children_count = calc_items_count(item.children);
            if (n_item <= count + children_count)
            {
                return get_item(item.children, n_item - 1, count, level + 1);
            }
            else
            {
                count += children_count;
            }
        }

        ++count;
    }

    return nullptr;
}

menu::menu(const std::string &theme_control_name, std::shared_ptr<i_theme> theme__)
    : list_theme(make_custom_theme()),
    list_(new list(list::tc, list_theme)),
    tcn(theme_control_name),
    theme_(theme__),
    position_(),
    parent_(),
    my_subscriber_id(),
    activation_control(),
    indent(0), x(-1), y(-1),
    items(),
    max_text_width(0), max_hotkey_width(0),
    showed_(false),
    size_updated(false)
{
    update_list_theme();

    list_->set_draw_callback(std::bind(&menu::draw_list_item, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4, std::placeholders::_5));
    list_->set_item_click_callback(std::bind(&menu::activate_list_item, this, std::placeholders::_1));
    list_->set_item_activate_callback(std::bind(&menu::activate_list_item, this, std::placeholders::_1));
    list_->set_mode(list::list_mode::auto_select);
}

menu::~menu()
{
    auto parent__ = parent_.lock();
    if (parent__)
    {
        parent__->remove_control(list_);
    }
}

void menu::draw(graphic &gr, const rect &)
{
}

void menu::set_position(const rect &position__, bool redraw)
{
    list_->set_position(position__, redraw);
}

rect menu::position() const
{
    return list_->position();
}

void menu::set_parent(std::shared_ptr<window> window)
{
    parent_ = window;

    if (window)
    {
        window->add_control(list_, { 0 });

        my_subscriber_id = window->subscribe(std::bind(&menu::receive_event, this, std::placeholders::_1), 
            static_cast<event_type>(static_cast<uint32_t>(event_type::mouse) | static_cast<uint32_t>(event_type::keyboard)));
    }
}

std::weak_ptr<window> menu::parent() const
{
    return parent_;
}

void menu::clear_parent()
{
    auto parent__ = parent_.lock();
    if (parent__)
    {
        list_->clear_parent();
        parent__->unsubscribe(my_subscriber_id);
    }

    parent_.reset();
}

void menu::update_list_theme()
{
    list_theme->load_theme(theme_ ? *theme_ : *get_default_theme());

    list_theme->set_color(list::tc, list::tv_background, theme_color(tcn, tv_background, theme_));
    list_theme->set_color(list::tc, list::tv_border, theme_color(tcn, tv_border, theme_));
    list_theme->set_color(list::tc, list::tv_focused_border, theme_color(tcn, tv_border, theme_));
    list_theme->set_dimension(list::tc, list::tv_border_width, theme_dimension(tcn, tv_border_width, theme_));
    list_theme->set_color(list::tc, list::tv_scrollbar, theme_color(tcn, tv_scrollbar, theme_));
    list_theme->set_color(list::tc, list::tv_scrollbar_slider, theme_color(tcn, tv_scrollbar_slider, theme_));
    list_theme->set_color(list::tc, list::tv_scrollbar_slider_acive, theme_color(tcn, tv_scrollbar_slider_acive, theme_));
    list_theme->set_dimension(list::tc, list::tv_round, theme_dimension(tcn, tv_round, theme_));
}

void menu::receive_event(const event &ev)
{
    if (!list_->showed())
    {
        return;
    }

    switch (ev.type)
    {
        case event_type::mouse:
            if ((ev.mouse_event_.type == mouse_event_type::left_up || ev.mouse_event_.type == mouse_event_type::right_up) &&
                !list_->position().in({ ev.mouse_event_.x, ev.mouse_event_.y, ev.mouse_event_.x, ev.mouse_event_.y }) &&
                (!activation_control ||
                (activation_control && !activation_control->position().in({ ev.mouse_event_.x, ev.mouse_event_.y, ev.mouse_event_.x, ev.mouse_event_.y }))))
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

void menu::set_topmost(bool)
{
}

bool menu::topmost() const
{
    return true;
}

bool menu::focused() const
{
    return false;
}

bool menu::focusing() const
{
    return false;
}

void menu::update_theme(std::shared_ptr<i_theme> theme__)
{
    if (theme_ && !theme__)
    {
        return;
    }
    theme_ = theme__;

    update_list_theme();

    size_updated = false;
}

void menu::show()
{
    if (showed_)
    {
        return;
    }

    showed_ = true;

    list_->show();
}

void menu::hide()
{
    showed_ = false;

    list_->hide();
}

bool menu::showed() const
{
    return showed_;
}

void menu::enable()
{
}

void menu::disable()
{
}

bool menu::enabled() const
{
    return true;
}

void menu::set_items(const menu_items_t &items_)
{
    items = items_;
    list_->set_item_count(calc_items_count(items));
}

void menu::update_item(const menu_item &mi)
{
    auto it = std::find(items.begin(), items.end(), mi.id);
    if (it != items.end())
    {
        *it = mi;
    }
    list_->set_item_count(calc_items_count(items));
}

void menu::swap_items(int32_t first_item_id, int32_t second_item_id)
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
    list_->set_item_count(calc_items_count(items));
}

void menu::delete_item(int32_t id)
{
    auto it = std::find(items.begin(), items.end(), id);
    if (it != items.end())
    {
        items.erase(it);
        size_updated = false;
    }
    list_->set_item_count(calc_items_count(items));
}

void menu::set_item_height(int32_t item_height_)
{
    list_->set_item_height(item_height_);
    size_updated = false;
}

void menu::update_size()
{
    if (size_updated || items.empty())
    {
        return;
    }

    system_context ctx = { 0 };
    auto parent__ = parent_.lock();
    if (parent__)
    {
#ifdef _WIN32
        ctx = { parent__->context().hwnd, GetDC(parent__->context().hwnd) };

        if (!ctx.hwnd)
        {
            return;
        }
#elif __linux__
        ctx = parent__->context();

        if (!ctx.display)
        {
            return;
        }
#endif
    }

    graphic mem_gr(ctx);
    mem_gr.init({ 0, 0, 1024, 500 }, 0);

    auto font_ = theme_font(tcn, tv_font, theme_);

    max_text_width = 0, max_hotkey_width = 0;

    auto items_count = calc_items_count(items);
    for (int i = 0; i != items_count; ++i)
    {
        auto *item = get_item(items, i);
        if (!item)
        {
            continue;
        }

        auto text_width = mem_gr.measure_text(item->text, font_).right;
        auto hotkey_width = mem_gr.measure_text(item->hotkey, font_).right;
        if (hotkey_width != 0)
        {
            hotkey_width += list_->get_item_height();
        }
        if (hotkey_width > max_hotkey_width)
        {
            max_hotkey_width = hotkey_width;
        }
        
        auto width = (item->level * list_->get_item_height()) + text_width + max_hotkey_width + (list_->get_item_height() * 3);
        if (width > max_text_width)
        {
            max_text_width = width;
        }
    }

    int32_t height = list_->get_item_height() * items_count;

    position_ = { 0, 0, max_text_width, height };

#ifdef _WIN32
    ReleaseDC(ctx.hwnd, ctx.dc);
#endif

    size_updated = true;
}

void menu::show_on_control(std::shared_ptr<i_control> control, int32_t indent_, int32_t x_, int32_t y_)
{
    activation_control = control;
    indent = indent_;
    x = x_;
    y = y_;

    update_size();

    auto base_pos = control ? control->position() : rect{ 0 };

    if (x != -1)
    {
        base_pos.put(x, base_pos.top);
    }
    if (y != -1)
    {
        base_pos.put(base_pos.left, y);
    }

    auto pos = get_popup_position(parent_, base_pos, position_, indent, false);

    list_->set_position(pos, true);
    list_->show();
    
    auto parent__ = parent_.lock();
    if (parent__)
    {
        parent__->set_focused(list_);
    }
}

void menu::show_on_point(int32_t x, int32_t y)
{
    show_on_control(nullptr, 0, x, y);
}

void menu::draw_arrow_down(graphic &gr, rect pos, bool expanded)
{
    auto color = theme_color(tcn, !expanded ? tv_text : tv_scrollbar_slider_acive, theme_);

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

void menu::draw_list_item(graphic &gr, int32_t n_item, const rect &item_rect_, list::item_state state, const std::vector<list::column> &columns)
{
    auto item = get_item(items, n_item);
    if (!item)
    {
        return;
    }

    auto border_width = theme_dimension(tcn, tv_border_width);

    auto item_rect = item_rect_;

    if (item_rect.bottom > list_->position().bottom - border_width)
    {
        item_rect.bottom = list_->position().bottom - border_width;
    }

    if (state == list::item_state::selected)
    {
        gr.draw_rect({ item_rect.left, item_rect.top + 1, item_rect.right, item_rect.bottom - 1 }, theme_color(tcn, tv_selected_item));
    }
    
    if (item->image_)
    {
        auto img_size = static_cast<int32_t>(item_rect_.height() * 0.9);

        auto indent = static_cast<int32_t>((item_rect_.height() - img_size) / 2);

        rect img_rect = { item_rect_.left + indent,
            item_rect_.top + indent,
            item_rect_.left + img_size + indent,
            item_rect_.top + img_size + indent };

        item->image_->set_position(img_rect);
        item->image_->draw(gr, { 0 });
    }

    auto text_color = item->state != menu_item_state::disabled ? theme_color(tcn, tv_text) : theme_color(tcn, tv_disabled_text);
    auto font = theme_font(tcn, tv_font);

    auto text_height = gr.measure_text("Qq", font).height();
    if (text_height <= item_rect.height())
    {
        gr.draw_text({ item_rect.left + item_rect.height() + item_rect.height() * item->level, item_rect_.top + (item_rect_.height() - text_height) / 2 }, item->text, text_color, font);

        if (!item->hotkey.empty())
        {
            gr.draw_text({ item_rect.right - max_hotkey_width, item_rect_.top + (item_rect_.height() - text_height) / 2 }, item->hotkey, text_color, font);
        }

        if (!item->children.empty())
        {
            auto height = item_rect_.height();

            auto left = item_rect_.right - item_rect_.height() + (height - 8) / 2,
                top = item_rect_.top + (height - 4) / 2;

            draw_arrow_down(gr, { left, top }, item->state == menu_item_state::expanded);
        }
    }

    if (item->state == menu_item_state::separator && item_rect_.bottom <= list_->position().bottom - border_width)
    {
        gr.draw_line({ item_rect_.left, item_rect_.bottom, item_rect_.right, item_rect_.bottom }, text_color);
    }
}

void menu::activate_list_item(int32_t n_item)
{
    auto item = get_item(items, n_item);
    if (!item)
    {
        return;
    }

    if (!item->children.empty())
    {
        if (item->state != menu_item_state::expanded)
        {
            item->prev_state = item->state;
            item->state = menu_item_state::expanded;
        }
        else
        {
            item->state = item->prev_state;
        }
        size_updated = false;
        list_->set_item_count(calc_items_count(items));
        show_on_control(activation_control, indent, x, y);
    }
    
    if (item->children.empty() && item->state != menu_item_state::disabled)
    {
        list_->hide();
    }

    if (item->click_callback && item->state != menu_item_state::disabled)
    {
        item->click_callback(n_item);
    }
}

}
