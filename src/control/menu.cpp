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

namespace wui
{

menu::menu(const std::string &text_, std::shared_ptr<i_theme> theme__)
    : theme_(theme__),
    position_(),
    parent(),
    showed_(false)
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

    //gr.draw_text(text_position, text, theme_color(tc, tv_text, theme_), font_);
}

void menu::receive_event(const event &)
{
}

void menu::set_position(const rect &position__)
{
    auto prev_position = position_;
    position_ = position__;

    update_size();

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

    update_size();

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

void menu::update_size()
{
    //if (text.empty())
    {
        //return;
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
    mem_gr.init(rect{ 0, 0, 1024, 50 }, 0);

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

void menu::show_on_control(i_control &control)
{
    auto parent_ = parent.lock();
    if (!parent_)
    {
        return;
    }

    auto parent_pos = parent_->position();

    update_size();

    auto out_pos = position_;

    out_pos.put(control.position().left + 5, control.position().bottom + 5); // below the control
    if (out_pos.bottom <= parent_pos.height())
    {
        if (out_pos.right >= parent_pos.width())
        {
            out_pos.put(parent_pos.width() - out_pos.width(), control.position().bottom + 5);
        }
    }
    else
    {
        out_pos.put(control.position().left + 5, control.position().top - out_pos.height() - 5); // above the control

        if (out_pos.right >= parent_pos.width())
        {
            out_pos.put(parent_pos.width() - out_pos.width(), control.position().top - out_pos.height() - 5);
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
