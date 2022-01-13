//
// Copyright (c) 2021-2022 Anton Golovkov (udattsk at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/ud84/WUI
//

#include <wui/control/button.hpp>

#include <wui/window/window.hpp>

#include <wui/control/image.hpp>
#include <wui/control/tooltip.hpp>

#include <wui/theme/theme.hpp>

#include <wui/system/tools.hpp>

#include <wui/system/char_encoding.hpp>

namespace wui
{

button::button(const std::wstring &caption_, std::function<void(void)> click_callback_, std::shared_ptr<i_theme> theme__)
    : button_view_(button_view::only_text),
    caption(caption_),
    image_(),
    image_size(0),
    tooltip_(new tooltip(caption_, theme__)),
    click_callback(click_callback_),
    theme_(theme__),
    position_(),
    parent(),
    showed_(true), enabled_(true), active(false), focused_(false),
    focusing_(true)
{
}

#ifdef _WIN32
button::button(const std::wstring &caption_, std::function<void(void)> click_callback_, button_view button_view__, int32_t image_resource_index_, int32_t image_size_, std::shared_ptr<i_theme> theme__)
    : button_view_(button_view__),
    caption(caption_),
    image_(new image(image_resource_index_, theme__)),
    image_size(image_size_),
    tooltip_(new tooltip(caption_, theme__)),
    click_callback(click_callback_),
    theme_(theme__),
    position_(),
    parent(),
    showed_(true), enabled_(true), active(false), focused_(false),
    focusing_(true)
{
}
#endif

button::button(const std::wstring &caption_, std::function<void(void)> click_callback_, button_view button_view__, const std::wstring &imageFileName_, int32_t image_size_, std::shared_ptr<i_theme> theme__)
    : button_view_(button_view__),
    caption(caption_),
    image_(new image(imageFileName_, theme__)),
    image_size(image_size_),
    tooltip_(new tooltip(caption_, theme__)),
    click_callback(click_callback_),
    theme_(theme__),
    position_(),
    parent(),
    showed_(true), enabled_(true), active(false), focused_(false),
    focusing_(true)
{
}

button::~button()
{
    auto parent_ = parent.lock();
    if (parent_)
    {
        parent_->remove_control(shared_from_this());
    }
}

void button::draw(graphic &gr)
{
    if (!showed_)
    {
        return;
    }

    auto font_ = font_settings{ theme_string(theme_value::button_font_name, theme_),
        theme_dimension(theme_value::button_font_size, theme_),
        font_decorations::normal };

    auto text_rect = gr.measure_text(caption, font_);

    int32_t text_top = 0, text_left = 0, image_left = 0, image_top = 0;

    switch (button_view_)
    {
        case button_view::only_text:
            if (text_rect.right + 10 > position_.width())
            {
                position_.right = position_.left + text_rect.right + 10;
            }

            text_top = position_.top + ((position_.height() - text_rect.bottom) / 2);
            text_left = position_.left + ((position_.width() - text_rect.right) / 2);
        break;
        case button_view::only_image:
            if (image_)
	        {
                if (image_size > position_.width())
                {
                    position_.right = position_.left + image_size;
                }
                if (image_size > position_.height())
                {
                    position_.bottom = position_.top + image_size;
                }

                image_top = position_.top + ((position_.height() - image_size) / 2);
                image_left = position_.left + ((position_.width() - image_size) / 2);
            }
        break;
        case button_view::image_right_text: case button_view::image_right_text_no_frame:
            if (image_)
            {
                if (image_size + text_rect.right + 6 > position_.width())
                {
                    position_.right = position_.left + text_rect.right + image_size + 6;
                }
                if (image_size + 6 > position_.height())
                {
                    position_.bottom = position_.top + image_size + 6;
                }

                text_top = position_.top + ((position_.height() - text_rect.bottom) / 2);
                image_left = position_.left + ((position_.width() - text_rect.right - image_size - 5) / 2);
                image_top = position_.top + ((position_.height() - image_size) / 2);
                text_left = image_left + image_size + 5;
            }
        break;
        case button_view::image_bottom_text:
            if (image_)
            {
                if (image_size + 6 > position_.width())
                {
                    position_.right = position_.left + image_size + 6;
                }
                if (image_size + text_rect.bottom + 6 > position_.height())
                {
                    position_.bottom = position_.top + text_rect.bottom + image_size + 6;
                }

                image_top = position_.top + ((position_.height() - text_rect.bottom - image_size - 5) / 2);
                text_top = image_top + image_size + 5;
                text_left = position_.left + ((position_.width() - text_rect.right) / 2);
                image_left = position_.left + ((position_.width() - image_size) / 2);
            }
        break;
    }

    color border_color = !focused_ ?
        (button_view_ != button_view::image_right_text_no_frame ? theme_color(theme_value::button_border, theme_) : theme_color(theme_value::window_background, theme_)) : 
        theme_color(theme_value::button_focused_border, theme_);

    color fill_color = enabled_ ?
        (active ? (button_view_ != button_view::image_right_text_no_frame ? theme_color(theme_value::button_active, theme_) : theme_color(theme_value::window_background, theme_)) : 
        (button_view_ != button_view::image_right_text_no_frame ? theme_color(theme_value::button_calm, theme_) : theme_color(theme_value::window_background, theme_))) :
        button_view_ != button_view::image_right_text_no_frame ? theme_color(theme_value::button_disabled, theme_) : theme_color(theme_value::window_background, theme_);

    gr.draw_rect(position_, border_color, fill_color, 1, theme_dimension(theme_value::button_round, theme_));
	
    if (button_view_ != button_view::only_text && image_)
    {
        image_->set_position( { image_left, image_top, image_left + image_size, image_top + image_size } );
        image_->draw(gr);
    }

    if (button_view_ != button_view::only_image)
    {
        gr.draw_text(rect{ text_left, text_top, text_left, text_top }, caption, 
            button_view_ != button_view::image_right_text_no_frame ? theme_color(theme_value::button_text, theme_) : theme_color(theme_value::window_text, theme_),
            font_);
    }
}

void button::receive_event(const event &ev)
{
    if (!showed_ || !enabled_)
    {
        return;
    }

    if (ev.type == event_type::mouse)
    {
        switch (ev.mouse_event_.type)
        {
            case mouse_event_type::enter:
            {
                active = true;
                auto parent_ = parent.lock();
                if (parent_)
                {
                    set_cursor(parent_->context(), cursor::default_);
                }
                redraw();

                if (button_view_ == button_view::only_image && !caption.empty())
                {
                    update_tooltip_position();
                    tooltip_->show();
                }
            }
            break;
            case mouse_event_type::leave:
                if (button_view_ == button_view::only_image && !caption.empty())
                {
                    tooltip_->hide();
                }

                active = false;
                redraw();
            break;
            case mouse_event_type::left_up:
                if (click_callback && enabled_)
                {
                    click_callback();
                }
            break;
        }
    }
    else if (ev.type == event_type::internal)
    {
        if (ev.internal_event_.type == internal_event_type::execute_focused && click_callback)
        {
            click_callback();
        }
    }
}

void button::set_position(const rect &position__)
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

        redraw();
    }
}

rect button::position() const
{
    return position_;
}

void button::set_parent(std::shared_ptr<window> window_)
{
    parent = window_;
    window_->add_control(tooltip_, tooltip_->position());
}

void button::clear_parent()
{
    parent.reset();
}

bool button::topmost() const
{
    return false;
}

void button::set_focus()
{
    if (focusing_ && enabled_ && showed_)
    {
        focused_ = true;

        redraw();
    }
}

bool button::remove_focus()
{
    focused_ = false;

    redraw();

    return true;
}

bool button::focused() const
{
    return focused_;
}

bool button::focusing() const
{
    return enabled_ && showed_ && focusing_;
}

void button::update_theme(std::shared_ptr<i_theme> theme__)
{
    if (theme_ && !theme__)
    {
        return;
    }
    theme_ = theme__;

    tooltip_->update_theme(theme_);

    if (image_)
    {
        image_->update_theme(theme_);
    }
}

void button::show()
{
    showed_ = true;
    redraw();
}

void button::hide()
{
    showed_ = false;
    auto parent_ = parent.lock();
    if (parent_)
    {
        parent_->redraw(position_, true);
    }
}

bool button::showed() const
{
    return showed_;
}

void button::enable()
{
    enabled_ = true;
    redraw();
}

void button::disable()
{
    enabled_ = false;
    redraw();
}

bool button::enabled() const
{
    return enabled_;
}

void button::set_caption(const std::wstring &caption_)
{
    caption = caption_;
    tooltip_->set_text(caption_);
    
    redraw();
}

void button::set_button_view(button_view button_view__)
{
    button_view_ = button_view__;

    redraw();
}

#ifdef _WIN32
void button::set_image(int32_t resource_index)
{
    if (image_)
    {
        image_->change_image(resource_index);
    }
    else
    {
        image_ = std::shared_ptr<image>(new image(resource_index));
    }
    redraw();
}
#endif

void button::set_image(const std::wstring &file_name)
{
    if (image_)
    {
        image_->change_image(file_name);
    }
    else
    {
        image_ = std::shared_ptr<image>(new image(file_name));
    }
    redraw();
}

void button::enable_focusing()
{
    focusing_ = true;
}

void button::disable_focusing()
{
    focusing_ = false;
}

void button::set_callback(std::function<void(void)> click_callback_)
{
    click_callback = click_callback_;
}

void button::redraw()
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

void button::update_tooltip_position()
{
    auto parent_ = parent.lock();
    if (!parent_)
    {
        return;
    }

    auto parent_pos = parent_->position();

    auto tt_size = tooltip_->position();
    if (tt_size.width() == 0)
    {
        tooltip_->update_size();
        tt_size = tooltip_->position();
    }

    auto out_pos = tt_size;
    out_pos.put(position_.left + 5, position_.bottom + 5); // below the button
    if (out_pos.bottom <= parent_pos.bottom)
    {
        if (out_pos.right >= parent_pos.right)
        {
            out_pos.put(parent_pos.right - tt_size.width(), position_.bottom + 5);
        }
    }
    else
    {
        out_pos.put(position_.left + 5, position_.top - out_pos.height() - 5); // above the button

        if (out_pos.right >= parent_pos.right)
        {
            out_pos.put(parent_pos.right - tt_size.width(), position_.top - out_pos.height() - 5);
        }
    }

    tooltip_->set_position(out_pos);
}

}
