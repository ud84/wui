//
// Copyright (c) 2021-2022 Anton Golovkov (udattsk at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/ud84/wui
//

#include <wui/control/panel.hpp>

#include <wui/window/window.hpp>

#include <wui/theme/theme.hpp>

#include <wui/system/tools.hpp>

namespace wui
{

panel::panel(std::shared_ptr<i_theme> theme__)
    : theme_(theme__),
    position_(),
    parent(),
    showed_(true),
    draw_callback()
{
}

panel::panel(std::function<void(graphic&)> draw_callback_, std::shared_ptr<i_theme> theme__)
    : theme_(theme__),
    position_(),
    parent(),
    showed_(true),
    draw_callback(draw_callback_)
{
}

panel::~panel()
{
    auto parent_ = parent.lock();
    if (parent_)
    {
        parent_->remove_control(shared_from_this());
    }
}

void panel::draw(graphic &gr, const rect &)
{
    if (!showed_)
    {
        return;
    }

    gr.draw_rect(position(), theme_color(tc, tv_background, theme_));

    if (draw_callback)
    {
        draw_callback(gr);
    }
}

void panel::set_position(const rect &position__, bool redraw)
{
    update_control_position(position_, position__, showed_ && redraw, parent);
}

rect panel::position() const
{
    return get_control_position(position_, parent);
}

void panel::set_parent(std::shared_ptr<window> window)
{
    parent = window;
}

void panel::clear_parent()
{
    parent.reset();
}

bool panel::topmost() const
{
    return false;
}

void panel::set_focus()
{
}

bool panel::remove_focus()
{
    return true;
}

bool panel::focused() const
{
    return false;
}

bool panel::focusing() const
{
    return false;
}

void panel::update_theme(std::shared_ptr<i_theme> theme__)
{
    if (theme_ && !theme__)
    {
        return;
    }
    theme_ = theme__;

    redraw();
}

void panel::show()
{
    showed_ = true;
    redraw();
}

void panel::hide()
{
    showed_ = false;
    auto parent_ = parent.lock();
    if (parent_)
    {
        parent_->redraw(position(), true);
    }
}

bool panel::showed() const
{
    return showed_;
}

void panel::enable()
{
}

void panel::disable()
{
}

bool panel::enabled() const
{
    return true;
}

void panel::redraw()
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

}
