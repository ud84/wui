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
    showed_(false),
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

void tooltip::draw(graphic &gr)
{
    if (!showed_)
    {
        return;
    }

    gr.draw_rect(position_,
        theme_color(theme_value::tooltip_border, theme_),
        theme_color(theme_value::tooltip_background, theme_),
        1,
        theme_dimension(theme_value::tooltip_round, theme_));

    auto font_ = font_settings{ theme_string(theme_value::tooltip_font_name, theme_),
        theme_dimension(theme_value::tooltip_font_size, theme_),
        font_decorations::normal };

    auto text_indent = theme_dimension(theme_value::tooltip_text_indent, theme_);

    auto text_position = position_;
    text_position.move(text_indent, text_indent);

    gr.draw_text(text_position, text, theme_color(theme_value::tooltip_text, theme_), font_);
}

void tooltip::receive_event(const event &)
{
}

void tooltip::set_position(const rect &position__)
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

bool tooltip::topmost() const
{
    return true;
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

    update_size();

    redraw();
}

void tooltip::show()
{
    if (showed_)
    {
        return;
    }

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

void tooltip::set_text(const std::wstring &text_)
{
    text = text_;

    update_size();

    redraw();
}

void tooltip::update_size()
{
    if (text.empty())
    {
        return;
    }

#ifdef _WIN32
    system_context ctx = { 0, GetDC(NULL) };
#elif __linux__
    system_context ctx = { 0 };
    auto parent_ = parent.lock();
    if (parent_)
    {
        ctx = { parent_->context().display, parent_->context().connection, parent_->context().screen, parent_->context().wnd };
    }

    if (!ctx.display)
    {
        return;
    }

#endif
    graphic mem_gr(ctx);
    mem_gr.init(rect{ 0, 0, 1024, 50 }, 0);

    auto font_ = font_settings{ theme_string(theme_value::tooltip_font_name, theme_),
        theme_dimension(theme_value::tooltip_font_size, theme_),
        font_decorations::normal };

    auto old_position = position_;

    position_ = mem_gr.measure_text(text, font_);

    auto text_indent = theme_dimension(theme_value::tooltip_text_indent, theme_);
    position_.right += text_indent * 2;
    position_.bottom += text_indent * 2;

    position_.move(old_position.left, old_position.top);
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
