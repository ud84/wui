//
// Copyright (c) 2021-2022 Anton Golovkov (udattsk at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/ud84/wui
//

#include <wui/control/button.hpp>

#include <wui/window/window.hpp>

#include <wui/control/image.hpp>
#include <wui/control/tooltip.hpp>

#include <wui/theme/theme.hpp>

#include <wui/system/tools.hpp>

namespace wui
{

button::button(const std::string &caption_, std::function<void(void)> click_callback_, std::shared_ptr<i_theme> theme__)
    : button_view_(button_view::text),
    caption(caption_),
    image_(),
    image_size(0),
    tooltip_(new tooltip(caption_, theme__)),
    click_callback(click_callback_),
    theme_(theme__),
    position_(),
    parent(),
    my_subscriber_id(),
    showed_(true), enabled_(true), active(false), focused_(false),
    focusing_(true),
    text_rect{ 0 }
{
}

#ifdef _WIN32
button::button(const std::string &caption_, std::function<void(void)> click_callback_, button_view button_view__, int32_t image_resource_index_, int32_t image_size_, std::shared_ptr<i_theme> theme__)
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
    focusing_(true),
    text_rect{ 0 }
{
}
#endif

button::button(const std::string &caption_, std::function<void(void)> click_callback_, button_view button_view__, const std::string &imageFileName_, int32_t image_size_, std::shared_ptr<i_theme> theme__)
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
    focusing_(true),
    text_rect{ 0 }
{
}

button::button(const std::string &caption_, std::function<void(void)> click_callback_, button_view button_view__, const std::vector<uint8_t> &image_data, int32_t image_size_, std::shared_ptr<i_theme> theme__)
    : button_view_(button_view__),
    caption(caption_),
    image_(new image(image_data)),
    image_size(image_size_),
    tooltip_(new tooltip(caption_, theme__)),
    click_callback(click_callback_),
    theme_(theme__),
    position_(),
    parent(),
    showed_(true), enabled_(true), active(false), focused_(false),
    focusing_(true),
    text_rect{ 0 }
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

void button::draw(graphic &gr, const rect &)
{
    if (!showed_)
    {
        return;
    }

    auto font_ = theme_font(tc, tv_font, theme_);

    if (button_view_ != button_view::image && !caption.empty() && text_rect.width() == 0)
    {
        text_rect = gr.measure_text(caption, font_);
    }

    int32_t text_top = 0, text_left = 0, image_left = 0, image_top = 0;

    auto control_pos = position();

    switch (button_view_)
    {
        case button_view::text:
            if (text_rect.right + 10 > control_pos.width())
            {
                position_.right = control_pos.left + text_rect.right + 10;
                return redraw();
            }

            text_left = control_pos.left + ((control_pos.width() - text_rect.right) / 2);
            text_top = control_pos.top + ((control_pos.height() - text_rect.bottom) / 2);
        break;
        case button_view::image:
            if (image_)
	        {
                if (image_size > control_pos.width())
                {
                    position_.right = control_pos.left + image_size;
                    return redraw();
                }
                if (image_size > control_pos.height())
                {
                    position_.bottom = control_pos.top + image_size;
                    return redraw();
                }

                image_left = control_pos.left + ((control_pos.width() - image_size) / 2);
                image_top = control_pos.top + ((control_pos.height() - image_size) / 2);
            }
        break;
        case button_view::image_right_text: case button_view::image_right_text_no_frame:
            if (image_)
            {
                if (image_size + text_rect.right + 6 > position_.width())
                {
                    position_.right = control_pos.left + text_rect.right + image_size + 6;
                    return redraw();
                }
                if (image_size + 6 > control_pos.height())
                {
                    position_.bottom = control_pos.top + image_size + 6;
                    return redraw();
                }

                image_left = control_pos.left;
                image_top = control_pos.top + ((control_pos.height() - image_size) / 2);
                text_left = image_left + image_size + 5;
                text_top = control_pos.top + ((control_pos.height() - text_rect.bottom) / 2);
            }
        break;
        case button_view::image_bottom_text:
            if (image_)
            {
                if (image_size + 6 > control_pos.width())
                {
                    position_.right = control_pos.left + image_size + 6;
                    return redraw();
                }
                if (image_size + text_rect.bottom + 6 > control_pos.height())
                {
                    position_.bottom = control_pos.top + text_rect.bottom + image_size + 6;
                    return redraw();
                }

                text_left = control_pos.left + ((control_pos.width() - text_rect.right) / 2);
                text_top = image_top + image_size + 5;                
                image_left = control_pos.left + ((control_pos.width() - image_size) / 2);
                image_top = control_pos.top + ((control_pos.height() - text_rect.bottom - image_size - 5) / 2);
            }
        break;
    }

    if (button_view_ != button_view::image_right_text_no_frame)
    {
        color border_color = !focused_ ?
            (button_view_ != button_view::image_right_text_no_frame ? theme_color(tc, tv_border, theme_) : theme_color(window::tc, window::tv_background, theme_)) :
            theme_color(tc, tv_focused_border, theme_);

        color fill_color = enabled_ ?
            (active ? (button_view_ != button_view::image_right_text_no_frame ? theme_color(tc, tv_active, theme_) : theme_color(window::tc, window::tv_background, theme_)) :
            (button_view_ != button_view::image_right_text_no_frame ? theme_color(tc, tv_calm, theme_) : theme_color(window::tc, window::tv_background, theme_))) :
            button_view_ != button_view::image_right_text_no_frame ? theme_color(tc, tv_disabled, theme_) : theme_color(window::tc, window::tv_background, theme_);

        gr.draw_rect(control_pos, border_color, fill_color, theme_dimension(tc, tv_border_width, theme_), theme_dimension(tc, tv_round, theme_));
    }
	
    if (button_view_ != button_view::text && image_)
    {
        image_->set_position( { image_left, image_top, image_left + image_size, image_top + image_size }, false );
        image_->draw(gr, { 0 });
    }

    if (button_view_ != button_view::image)
    {
        gr.draw_text(rect{ text_left, text_top, text_left, text_top }, caption, 
            button_view_ != button_view::image_right_text_no_frame ? theme_color(tc, tv_text, theme_) : theme_color(window::tc, tv_text, theme_),
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

                if (button_view_ == button_view::image && !caption.empty())
                {
                    tooltip_->show_on_control(*this, 5);
                }
            }
            break;
            case mouse_event_type::leave:
                if (button_view_ == button_view::image && !caption.empty())
                {
                    tooltip_->hide();
                }

                active = false;
                redraw();
            break;
            case mouse_event_type::left_up:
                if (button_view_ == button_view::image && !caption.empty())
                {
                    tooltip_->hide();
                }

                active = false;
                redraw();

                if (click_callback && enabled_)
                {
                    click_callback();
                }
            break;
        }
    }
    else if (ev.type == event_type::internal)
    {
        switch (ev.internal_event_.type)
        {
            case internal_event_type::set_focus:
                if (focusing_ && enabled_ && showed_)
                {
                    focused_ = true;

                    redraw();
                }
            break;
            case internal_event_type::remove_focus:
                focused_ = false;
                redraw();
            break;
            case internal_event_type::execute_focused:
                if (click_callback)
                {
                    click_callback();
                }
            break;
        }
    }
}

void button::set_position(const rect &position__, bool redraw)
{
    update_control_position(position_, position__, showed_ && redraw, parent);
}

rect button::position() const
{
    return get_control_position(position_, parent);
}

void button::set_parent(std::shared_ptr<window> window_)
{
    parent = window_;
    window_->add_control(tooltip_, tooltip_->position());
    my_subscriber_id = window_->subscribe(std::bind(&button::receive_event, this, std::placeholders::_1),
        static_cast<event_type>(static_cast<uint32_t>(event_type::internal) | static_cast<uint32_t>(event_type::mouse)),
        shared_from_this());
}

void button::clear_parent()
{
    auto parent_ = parent.lock();
    if (parent_)
    {
        parent_->unsubscribe(my_subscriber_id);
    }
    parent.reset();
}

bool button::topmost() const
{
    return false;
}

bool button::focused() const
{
    return enabled_ && showed_ && focused_;
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

    redraw();
}

void button::show()
{
    if (!showed_)
    {
        showed_ = true;
        redraw();
    }
}

void button::hide()
{
    if (showed_)
    {
        showed_ = false;
        tooltip_->hide();
        auto parent_ = parent.lock();
        if (parent_)
        {
            parent_->redraw(position(), true);
        }
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

void button::set_caption(const std::string &caption_)
{
    caption = caption_;
    tooltip_->set_text(caption_);

    text_rect = { 0 };
    
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

void button::set_image(const std::string &file_name)
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

void button::set_image(const std::vector<uint8_t> &image_data)
{
    if (image_)
    {
        image_->change_image(image_data);
    }
    else
    {
        image_ = std::shared_ptr<image>(new image(image_data));
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
            parent_->redraw(position());
        }
    }
}

}
