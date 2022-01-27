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
    size_updated(false),
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

    gr.draw_rect(position_,
        theme_color(tc, tv_border, theme_),
        theme_color(tc, tv_background, theme_),
        theme_dimension(tc, tv_border_width, theme_),
        theme_dimension(tc, tv_round, theme_));

    auto font_ = theme_font(tc, tv_font, theme_);

    auto text_indent = theme_dimension(tc, tv_text_indent, theme_);

    auto text_position = position_;
    text_position.move(text_indent, text_indent);

    gr.draw_text(text_position, text, theme_color(tc, tv_text, theme_), font_);
}

void tooltip::set_position(const rect &position__)
{
    update_control_position(position_, position__, showed_, parent);
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

    size_updated = false;

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

void tooltip::set_text(const std::string &text_)
{
    text = text_;

    size_updated = false;

    redraw();
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

    position_ = mem_gr.measure_text(text, font_);

    auto text_indent = theme_dimension(tc, tv_text_indent, theme_);
    position_.right += text_indent * 2;
    position_.bottom += text_indent * 2;

    position_.move(old_position.left, old_position.top);

#ifdef _WIN32
    ReleaseDC(ctx.hwnd, ctx.dc);
#endif
}

void tooltip::show_on_control(i_control &control, int32_t indent)
{
    auto parent_ = parent.lock();
    if (!parent_)
    {
        return;
    }

    if (!size_updated)
    {
        update_size();
        size_updated = true;
    }

    auto parent_pos = parent_->position();
    if (!parent_->child())
    {
        parent_pos = { 0, 0, parent_pos.width(), parent_pos.height() };
    }

    auto control_pos = control.position();

    auto out_pos = position_;

    out_pos.put(control_pos.left + indent, control_pos.top + indent); // on the control

    bool position_finded = false;

    out_pos.put(control_pos.left + indent, control_pos.bottom + indent); // below the control
    if (out_pos.bottom <= parent_pos.bottom)
    {
        if (out_pos.right >= parent_pos.width())
        {
            out_pos.put(parent_pos.width() - out_pos.width(), control_pos.bottom + indent);
        }
        position_finded = true;
    }
    
    if (!position_finded)
    {
        out_pos.put(control_pos.left + indent, control_pos.top - out_pos.height() - indent); // above the control
        if (out_pos.top >= parent_pos.top)
        {
            if (out_pos.right >= parent_pos.width())
            {
                out_pos.put(parent_pos.width() - out_pos.width(), control_pos.top - out_pos.height() - indent);
            }
            position_finded = true;
        }
    }

    if (!position_finded)
    {
        out_pos.put(control_pos.right + indent, control_pos.top + indent); // to the right of the control
        if (out_pos.right <= parent_pos.width())
        {
            position_finded = true;
        }
    }

    if (!position_finded)
    {
        out_pos.put(control_pos.left - out_pos.width() - indent, control_pos.top + indent); // to the left of the control
        if (out_pos.left >= parent_pos.left)
        {
            position_finded = true;
        }
    }

    if (!position_finded)
    {
        out_pos.put(control_pos.left + indent, control_pos.top + indent); // on the control
    }

    set_position(out_pos);
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
