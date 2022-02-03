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

menu::menu(std::shared_ptr<i_theme> theme__)
    : list_theme(make_custom_theme()),
    list_(new list(theme__)),
    theme_(theme__),
    position_(),
    parent(),
    items(),
    item_height(20),
    max_width(-1),
    showed_(false),
    size_updated(false)
{
}

menu::~menu()
{
    auto parent_ = parent.lock();
    if (parent_)
    {
        parent_->remove_control(list_);
    }
}

void menu::draw(graphic &gr, const rect &)
{   
}

void menu::set_position(const rect &position__, bool redraw)
{
    update_control_position(position_, position__, showed_ && redraw, parent);
}

rect menu::position() const
{
    return get_control_position(position_, parent);
}

void menu::set_parent(std::shared_ptr<window> window)
{
    list_->set_parent(window);
    parent = window;
}

void menu::clear_parent()
{
    list_->clear_parent();
    parent.reset();
}

bool menu::topmost() const
{
    return true;
}

void menu::set_focus()
{
}

bool menu::remove_focus()
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

    //list_theme->load_theme(*theme__);

    //list_->update_theme(list_theme);

    size_updated = false;

    redraw();
}

void menu::show()
{
    if (showed_)
    {
        return;
    }

    showed_ = true;

    redraw();
}

void menu::hide()
{
    showed_ = false;
    auto parent_ = parent.lock();
    if (parent_)
    {
        parent_->redraw(position_, true);
    }
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
}

void menu::update_item(const menu_item &mi, int32_t id)
{
    auto it = std::find(items.begin(), items.end(), mi.id);
    if (it != items.end())
    {
        *it = mi;
    }
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
}

void menu::delete_item(int32_t id)
{
    auto it = std::find(items.begin(), items.end(), id);
    if (it != items.end())
    {
        items.erase(it);
        size_updated = false;
    }
}

void menu::set_item_height(int32_t item_height_)
{
    item_height = item_height_;
    size_updated = false;
}

void menu::set_max_width(int32_t width)
{
    max_width = width;
    size_updated = false;
}

void menu::update_size()
{
    if (size_updated)// || items.empty())
    {
        return;
    }

    system_context ctx = { 0 };
    auto parent_ = parent.lock();
    if (parent_)
    {
#ifdef _WIN32
        ctx = { parent_->context().hwnd, GetDC(parent_->context().hwnd) };

        if (!ctx.hwnd)
        {
            return;
        }
#elif __linux__
        ctx = parent_->context();

        if (!ctx.display)
        {
            return;
        }
#endif
    }

    /*graphic mem_gr(ctx);
    mem_gr.init(rect{ 0, 0, 1024, 500 }, 0);

    auto font_ = theme_font(tc, tv_font, theme_);

    auto old_position = position_;*/

    position_ = { 0, 0, 100, 100 }; //mem_gr.measure_text(text, font_);

    /*auto text_indent = 5;
    position_.right += text_indent * 2;
    position_.bottom += text_indent * 2;

    position_.move(old_position.left, old_position.top);*/

#ifdef _WIN32
    ReleaseDC(ctx.hwnd, ctx.dc);
#endif

    size_updated = true;
}

void menu::show_on_control(i_control &control, int32_t indent)
{
    update_size();

    auto pos = get_best_position_on_control(parent, control.position(), position_, indent);

    list_->set_position(pos, false);
    list_->show();
}

void menu::redraw()
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

}
