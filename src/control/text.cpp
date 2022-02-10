//
// Copyright (c) 2021-2022 Anton Golovkov (udattsk at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/ud84/wui
//

#include <wui/control/text.hpp>

#include <wui/window/window.hpp>

#include <wui/theme/theme.hpp>

#include <wui/system/tools.hpp>

#include <cstring>

namespace wui
{

text::text(const std::string &text__, std::shared_ptr<i_theme> theme_)
    : theme_(theme_),
    position_(),
    parent(),
    showed_(true),
    text_(text__)
{
}

text::~text()
{
    auto parent_ = parent.lock();
    if (parent_)
    {
        parent_->remove_control(shared_from_this());
    }
}

void text::draw(graphic &gr, const rect &)
{
    if (!showed_)
    {
        return;
    }

    auto text__ = text_;

    auto font_ = theme_font(tc, tv_font, theme_);

    auto text_size = gr.measure_text(text__, font_);

    auto line_count = static_cast<double>(position_.height()) / static_cast<double>(text_size.height() + text_size.height() * 0.8);
    if (line_count > 1)
    {
        while (!text__.empty() && text_size.width() > position_.width())
        {
            text__.resize(text__.size() - 1);
            text_size = gr.measure_text(text__, font_);
        }
    }

    gr.draw_text(position(), text__, theme_color(tc, tv_color, theme_), font_);
}

void text::set_position(const rect &position__, bool redraw)
{
    update_control_position(position_, position__, showed_ && redraw, parent);
}

rect text::position() const
{
    return get_control_position(position_, parent);
}

void text::set_parent(std::shared_ptr<window> window)
{
    parent = window;
}

void text::clear_parent()
{
    parent.reset();
}

bool text::topmost() const
{
    return false;
}

void text::set_focus()
{
}

bool text::remove_focus()
{
    return true;
}

bool text::focused() const
{
    return false;
}

bool text::focusing() const
{
    return false;
}

void text::update_theme(std::shared_ptr<i_theme> theme__)
{
    if (theme_ && !theme__)
    {
        return;
    }
    theme_ = theme__;

    redraw();
}

void text::show()
{
    showed_ = true;
    redraw();
}

void text::hide()
{
    showed_ = false;
    auto parent_ = parent.lock();
    if (parent_)
    {
        parent_->redraw(position(), true);
    }
}

bool text::showed() const
{
    return showed_;
}

void text::enable()
{
}

void text::disable()
{
}

bool text::enabled() const
{
    return true;
}

void text::set_text(const std::string &text__)
{
    text_ = text__;
    redraw();
}

void text::redraw()
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
