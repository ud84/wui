//
// Copyright (c) 2021-2025 Anton Golovkov (udattsk at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://gitverse.ru/udattsk/wui
//

#include <wui/control/progress.hpp>

#include <wui/window/window.hpp>

#include <wui/theme/theme.hpp>

#include <wui/system/tools.hpp>

#include <boost/nowide/convert.hpp>

#include <cstring>

namespace wui
{

progress::progress(int32_t from_, int32_t to_, int32_t value_, orientation orientation__, std::string_view theme_control_name, std::shared_ptr<i_theme> theme__)
    : tcn(theme_control_name),
    theme_(theme__),
    position_(),
    parent_(),
    showed_(true), topmost_(false),
    from(from_),
    to(to_),
    value(value_),
    orientation_(orientation__)
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

void progress::draw(graphic &gr, rect )
{
    if (!showed_)
    {
        return;
    }

    auto control_pos = position();

    gr.draw_rect(control_pos,
        0,
        theme_color(tcn, tv_background, theme_),
        0,
        0);

    double total = orientation_ == orientation::horizontal ? control_pos.width() : control_pos.height();

    double meter_pos = (total * static_cast<double>(value)) / static_cast<double>(to - from);

    if (orientation_ == orientation::horizontal)
    {
        gr.draw_rect({ control_pos.left,
                control_pos.top,
                control_pos.left + static_cast<int32_t>(meter_pos),
                control_pos.bottom },
            theme_color(tc, tv_meter, theme_));
    }
    else if (orientation_ == orientation::vertical)
    {
        gr.draw_rect({ control_pos.left,
                control_pos.bottom,
                control_pos.right,
                control_pos.bottom - static_cast<int32_t>(meter_pos) },
            theme_color(tc, tv_meter, theme_));
    }

    auto border_width = theme_dimension(tcn, tv_border_width, theme_);
    gr.draw_rect(control_pos,
        theme_color(tcn, tv_border, theme_),
        make_color(0, 0, 0, 255),
        border_width,
        theme_dimension(tcn, tv_round, theme_));
}

void progress::set_position(rect position__)
{
    position_ = position__;
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

error progress::get_error() const
{
    return {};
}

void progress::update_theme_control_name(std::string_view theme_control_name)
{
    tcn = theme_control_name;
    update_theme(theme_);
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
        auto pos = position();
        pos.widen(theme_dimension(tcn, tv_border_width, theme_));
        parent__->redraw(pos, true);
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
            auto pos = position();
            pos.widen(theme_dimension(tcn, tv_border_width, theme_));
            parent__->redraw(pos);
        }
    }
}

}
