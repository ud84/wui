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

#include <wui/system/tools.hpp>

namespace wui
{

tooltip::tooltip(const std::string &text_, std::shared_ptr<i_theme> theme__)
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

void tooltip::draw(graphic &gr, const rect &)
{
    if (!showed_)
    {
        return;
    }

    auto control_position = position();

    gr.draw_rect(control_position,
        theme_color(tc, tv_border, theme_),
        theme_color(tc, tv_background, theme_),
        theme_dimension(tc, tv_border_width, theme_),
        theme_dimension(tc, tv_round, theme_));

    auto font_ = theme_font(tc, tv_font, theme_);

    auto text_indent = theme_dimension(tc, tv_text_indent, theme_);

    auto text_position = control_position;
    text_position.move(text_indent, text_indent);

    gr.draw_text(text_position, text, theme_color(tc, tv_text, theme_), font_);
}

void tooltip::set_position(const rect &position__, bool redraw)
{
    update_control_position(position_, position__, showed_ && redraw, parent);
}

rect tooltip::position() const
{
    return get_control_position(position_, parent);
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
        parent_->redraw(position(), true);
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

void tooltip::set_text(const std::string &text_)
{
    text = text_;

    if (showed_)
    {
        update_size();

        redraw();
    }
}

void tooltip::update_size()
{
    if (text.empty())
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

    auto position__ = mem_gr.measure_text(text, font_);

    auto text_indent = theme_dimension(tc, tv_text_indent, theme_);
    position__.right += text_indent * 2;
    position__.bottom += text_indent * 2;

    position__.move(old_position.left, old_position.top);

    set_position(position__, true);

#ifdef _WIN32
    ReleaseDC(ctx.hwnd, ctx.dc);
#endif
}

void tooltip::show_on_control(i_control &control, int32_t indent)
{
    update_size();

    auto pos = get_popup_position(parent, control.position(), position_, indent, true);
    
    set_position(pos, false);
    show();
}

void tooltip::redraw()
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
