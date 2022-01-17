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

#include <wui/theme/theme.hpp>

#include <algorithm>

namespace wui
{

menu::menu(const std::string &text_, std::shared_ptr<i_theme> theme__)
    : theme_(theme__),
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
        parent_->remove_control(shared_from_this());
    }
}

void menu::draw(graphic &gr)
{
    if (!showed_)
    {
        return;
    }

    gr.draw_rect(position_,
        theme_color(tc, tv_border, theme_),
        theme_color(tc, tv_background, theme_),
        1,
        theme_dimension(tc, tv_round, theme_));

    auto font_ = theme_font(tc, tv_font, theme_);

    auto text_indent = 5;

    auto text_position = position_;
    text_position.move(text_indent, text_indent);

    for (auto &item : items)
    {
        gr.draw_text(text_position, item.text, theme_color(tc, tv_text, theme_), font_);
    }
}

void menu::receive_event(const event &)
{
}

void menu::set_position(const rect &position__)
{
    auto prev_position = position_;
    position_ = position__;

    if (showed_)
    {
        auto parent_ = parent.lock();
        if (parent_)
        {
            parent_->redraw(prev_position, true);
        }
    }
	
    redraw();
}

rect menu::position() const
{
	return position_;
}

void menu::set_parent(std::shared_ptr<window> window)
{
    parent = window;
}

void menu::clear_parent()
{
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

void menu::append_item(const menu_item &mi)
{
    auto it = std::find(items.begin(), items.end(), mi.id);
    if (it == items.end())
    {
        items.emplace_back(mi);
        size_updated = false;
    }
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
    if (items.empty())
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

    graphic mem_gr(ctx);
    mem_gr.init(rect{ 0, 0, 1024, 500 }, 0);

    auto font_ = theme_font(tc, tv_font, theme_);

    auto old_position = position_;

    //position_ = mem_gr.measure_text(text, font_);

    auto text_indent = 5;
    position_.right += text_indent * 2;
    position_.bottom += text_indent * 2;

    position_.move(old_position.left, old_position.top);

#ifdef _WIN32
    ReleaseDC(ctx.hwnd, ctx.dc);
#endif
}

void menu::show_on_control(i_control &control, int32_t relative)
{
    auto parent_ = parent.lock();
    if (!parent_)
    {
        return;
    }

    auto parent_pos = parent_->position();

    if (!size_updated)
    {
        update_size();
        size_updated = true;
    }

    auto out_pos = position_;

    if (relative > 0)
    {
        out_pos.put(control.position().left + relative, control.position().bottom + relative); // below the control
    }
    else
    {
        out_pos.put(control.position().left, control.position().top); // on the control
    }

    if (out_pos.bottom <= parent_pos.height())
    {
        if (out_pos.right >= parent_pos.width())
        {
            out_pos.put(parent_pos.width() - out_pos.width(), relative > 0 ? control.position().bottom + relative : control.position().top);
        }
    }
    else
    {
        if (relative > 0)
        {
            out_pos.put(control.position().left + relative, control.position().top - out_pos.height() - relative); // above the control
        }
        else
        {
            out_pos.put(control.position().left, control.position().bottom - out_pos.height()); // on the control
        }

        if (out_pos.right >= parent_pos.width())
        {
            out_pos.put(parent_pos.width() - out_pos.width(), relative > 0 ? control.position().top - out_pos.height() - relative : control.position().bottom - out_pos.height());
        }
    }

    set_position(out_pos);
    show();
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