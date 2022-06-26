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
#include <vector>
#include <sstream>

namespace wui
{

text::text(const std::string &text__, text_alignment alignment_, const std::string &theme_control_name, std::shared_ptr<i_theme> theme_)
    : tcn(theme_control_name),
    theme_(theme_),
    position_(),
    parent_(),
    showed_(true), topmost_(false),
    text_(text__),
    alignment(alignment_)
{
}

text::~text()
{
    auto parent__ = parent_.lock();
    if (parent__)
    {
        parent__->remove_control(shared_from_this());
    }
}

void text::draw(graphic &gr, const rect &)
{
    if (!showed_)
    {
        return;
    }

    auto font_ = theme_font(tcn, tv_font, theme_);

    std::stringstream text__(text_);
    std::string line;
    std::vector<std::string> lines;

    while (std::getline(text__, line, '\n'))
    {
        lines.push_back(line);
    }

    auto control_pos = position();

    int32_t line_top = control_pos.top;

    auto text_height = gr.measure_text("Qq", font_).height();

    for (auto &line : lines)
    {
        truncate_line(line, gr, font_, control_pos.width());

        int32_t left = control_pos.left;

        switch (alignment)
        {
            case text_alignment::left:
                // do nothing
            break;
            case text_alignment::center:
            {
                auto line_width = gr.measure_text(line, font_).width();
                left += ((control_pos.width() - line_width) / 2);
            }
            break;
            case text_alignment::right:
            {
                auto line_width = gr.measure_text(line, font_).width();
                left += (control_pos.width() - line_width);
            }
            break;
        }
        
        gr.draw_text({ left, line_top }, line, theme_color(tcn, tv_color, theme_), font_);

        line_top += static_cast<int32_t>(text_height * 1.2);

        if (line_top + text_height > control_pos.bottom)
        {
            break;
        }
    }
}

void text::set_position(const rect &position__, bool redraw)
{
    update_control_position(position_, position__, showed_ && redraw, parent_);
}

rect text::position() const
{
    return get_control_position(position_, parent_);
}

void text::set_parent(std::shared_ptr<window> window)
{
    parent_ = window;
}

std::weak_ptr<window> text::parent() const
{
    return parent_;
}

void text::clear_parent()
{
    parent_.reset();
}

void text::set_topmost(bool yes)
{
    topmost_ = yes;
}

bool text::topmost() const
{
    return topmost_;
}

bool text::focused() const
{
    return false;
}

bool text::focusing() const
{
    return false;
}

void text::update_theme_control_name(const std::string &theme_control_name)
{
    tcn = theme_control_name;
    update_theme(theme_);
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
    auto parent__ = parent_.lock();
    if (parent__)
    {
        parent__->redraw(position(), true);
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

const std::string &text::get_text() const
{
	return text_;
}

void text::set_alignment(text_alignment alignment_)
{
    alignment = alignment_;
    redraw();
}

void text::redraw()
{
    if (showed_)
    {
        auto parent__ = parent_.lock();
        if (parent__)
        {
            parent__->redraw(position(), true);
        }
    }
}

}
