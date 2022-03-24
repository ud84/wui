//
// Copyright (c) 2021-2022 Anton Golovkov (udattsk at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/ud84/wui
//

#include <wui/control/progress.hpp>

#include <wui/window/window.hpp>

#include <wui/theme/theme.hpp>

#include <wui/system/tools.hpp>

#include <boost/nowide/convert.hpp>

#include <cstring>

namespace wui
{

progress::progress(int32_t from_, int32_t to_, int32_t value_, progress_orientation orientation_, const std::string &theme_control_name, std::shared_ptr<i_theme> theme__)
    : theme_(theme__),
    position_(),
    parent_(),
    showed_(true), topmost_(false),
    from(from_),
    to(to_),
    value(value_),
    orientation(orientation_)
{
}

progress::~progress()
{
    auto parent__ = parent_.lock();
    if (parent__)
    {
        parent__->remove_control(shared_from_this());
    }
}

void progress::draw(graphic &gr, const rect &)
{
    if (!showed_)
    {
        return;
    }

    auto control_pos = position();

    auto border_width = theme_dimension(tc, tv_border_width, theme_);

    gr.draw_rect(control_pos,
        theme_color(tc, tv_border, theme_),
        theme_color(tc, tv_background, theme_),
        border_width,
        0);

    double total = orientation == progress_orientation::horizontal ? control_pos.width() - border_width * 2 : control_pos.height() - border_width * 2;

    double meter_pos = (total * static_cast<double>(value)) / static_cast<double>(to - from);

    if (orientation == progress_orientation::horizontal)
    {
        gr.draw_rect({ control_pos.left + border_width,
                control_pos.top + border_width,
                control_pos.left + border_width + static_cast<int32_t>(meter_pos),
                control_pos.bottom - border_width },
            theme_color(tc, tv_meter, theme_));
    }
    else if (orientation == progress_orientation::vertical)
    {
        gr.draw_rect({ control_pos.left + border_width,
                control_pos.bottom - border_width,
                control_pos.right - border_width,
                control_pos.bottom - border_width - static_cast<int32_t>(meter_pos) },
            theme_color(tc, tv_meter, theme_));
    }
}

void progress::set_position(const rect &position__, bool redraw)
{
    update_control_position(position_, position__, showed_ && redraw, parent_);
}

rect progress::position() const
{
    return get_control_position(position_, parent_);
}

void progress::set_parent(std::shared_ptr<window> window)
{
    parent_ = window;
}

std::weak_ptr<window> progress::parent() const
{
    return parent_;
}

void progress::clear_parent()
{
    parent_.reset();
}

void progress::set_topmost(bool yes)
{
    topmost_ = yes;
}

bool progress::topmost() const
{
    return topmost_;
}

bool progress::focused() const
{
    return false;
}

bool progress::focusing() const
{
    return false;
}

void progress::update_theme(std::shared_ptr<i_theme> theme__)
{
    if (theme_ && !theme__)
    {
        return;
    }
    theme_ = theme__;

    redraw();
}

void progress::show()
{
    showed_ = true;
    redraw();
}

void progress::hide()
{
    showed_ = false;
    auto parent__ = parent_.lock();
    if (parent__)
    {
        parent__->redraw(position(), true);
    }
}

bool progress::showed() const
{
    return showed_;
}

void progress::enable()
{
}

void progress::disable()
{
}

bool progress::enabled() const
{
    return true;
}

void progress::set_range(int32_t from_, int32_t to_)
{
    from = from_;
    to = to_;

    redraw();
}

void progress::set_value(int32_t value_)
{
    value = value_;

    redraw();
}

void progress::redraw()
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

}
