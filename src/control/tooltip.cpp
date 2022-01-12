//
// Copyright (c) 2021-2022 Anton Golovkov (udattsk at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/ud84/wui
//

#include <wui/control/tooltip.hpp>

#include <wui/window/window.hpp>

#include <wui/theme/theme.hpp>

namespace wui
{

tooltip::tooltip(const std::wstring &text_, std::shared_ptr<i_theme> theme__)
    : theme_(theme__),
    position_(),
    parent(),
    showed_(true),
    text(text_)
{
}

tooltip::~tooltip()
{
    auto parent_ = parent.lock();
    if (parent_)
    {
        parent_->remove_control(shared_from_this());
    }
}

void tooltip::draw(graphic &gr_)
{
    if (!showed_)
    {
        return;
    }

}

void tooltip::receive_event(const event &)
{
}

void tooltip::set_position(const rect &position__)
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

rect tooltip::position() const
{
	return position_;
}

void tooltip::set_parent(std::shared_ptr<window> window)
{
    parent = window;
}

void tooltip::clear_parent()
{
    parent.reset();
}

void tooltip::set_focus()
{
}

bool tooltip::remove_focus()
{
    return true;
}

bool tooltip::focused() const
{
    return false;
}

bool tooltip::focusing() const
{
    return false;
}

void tooltip::update_theme(std::shared_ptr<i_theme> theme__)
{
    if (theme_ && !theme__)
    {
        return;
    }
    theme_ = theme__;

}

void tooltip::show()
{
    showed_ = true;
    redraw();
}

void tooltip::hide()
{
    showed_ = false;
    auto parent_ = parent.lock();
    if (parent_)
    {
        parent_->redraw(position_, true);
    }
}

bool tooltip::showed() const
{
    return showed_;
}

void tooltip::enable()
{
}

void tooltip::disable()
{
}

bool tooltip::enabled() const
{
    return true;
}

void tooltip::redraw()
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
