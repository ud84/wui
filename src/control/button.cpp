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

#include <wui/theme/theme.hpp>

namespace wui
{

button::button(const std::wstring &caption_, std::function<void(void)> click_callback_, std::shared_ptr<i_theme> theme__)
    : button_view_(button_view::only_text),
    caption(caption_),
    image_(),
    image_size(0),
    click_callback(click_callback_),
    theme_(theme__),
    position_(),
    parent(),
    showed_(true), enabled_(true), active(false), focused_(false),
    focusing_(true)
#ifdef _WIN32
    , calm_brush(0), active_brush(0), disabled_brush(0),
    border_pen(0), focused_border_pen(0)
#endif
{
#ifdef _WIN32
    make_primitives();
#endif
}

#ifdef _WIN32
button::button(const std::wstring &caption_, std::function<void(void)> click_callback_, button_view button_view__, int32_t image_resource_index_, int32_t image_size_, std::shared_ptr<i_theme> theme__)
    : button_view_(button_view__),
    caption(caption_),
    image_(new image(image_resource_index_, theme_)),
    image_size(image_size_),
    click_callback(click_callback_),
    theme_(theme__),
    position_(),
    parent(),
    showed_(true), enabled_(true), active(false), focused_(false),
    focusing_(true),
    calm_brush(0), active_brush(0), disabled_brush(0),
    border_pen(0), focused_border_pen(0)
{
    make_primitives();
}
#endif

button::button(const std::wstring &caption_, std::function<void(void)> click_callback_, button_view button_view__, const std::wstring &imageFileName_, int32_t image_size_, std::shared_ptr<i_theme> theme__)
    : button_view_(button_view__),
    caption(caption_),
    image_(new image(imageFileName_, theme_)),
    image_size(image_size_),
    click_callback(click_callback_),
    theme_(theme__),
    position_(),
    parent(),
    showed_(true), enabled_(true), active(false), focused_(false),
    focusing_(true)
#ifdef _WIN32
    , calm_brush(0), active_brush(0), disabled_brush(0),
    border_pen(0), focused_border_pen(0)
#endif
{
#ifdef _WIN32
    make_primitives();
#endif
}

button::~button()
{
#ifdef _WIN32
    destroy_primitives();
#endif

    if (parent.lock())
    {
        parent.lock()->remove_control(shared_from_this());
    }
}

void button::draw(graphic &gr)
{
    if (!showed_)
    {
        return;
    }

#ifdef _WIN32
    RECT text_rect = { 0 };
    DrawTextW(gr.dc, caption.c_str(), static_cast<int32_t>(caption.size()), &text_rect, DT_CALCRECT);

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

    SelectObject(gr.dc, !focused_ ? border_pen : focused_border_pen);
    SelectObject(gr.dc, enabled_ ? (active ? active_brush : calm_brush) : disabled_brush);

    auto rnd = theme_dimension(theme_value::button_round, theme_);
    RoundRect(gr.dc, position_.left, position_.top, position_.right, position_.bottom, rnd, rnd);
	
    if (button_view_ != button_view::only_text && image_)
    {
        image_->set_position( { image_left, image_top, image_left + image_size, image_top + image_size } );
        image_->draw(gr);
    }

    if (button_view_ != button_view::only_image)
    {
        if (button_view_ != button_view::image_right_text_no_frame)
        {
            SetTextColor(gr.dc, theme_color(theme_value::button_text, theme_));
            SetBkColor(gr.dc, enabled_ ? (active ? theme_color(theme_value::button_active, theme_) : theme_color(theme_value::button_calm, theme_)) : theme_color(theme_value::button_disabled, theme_));
        }
        else
        {
            SetTextColor(gr.dc, theme_color(theme_value::window_text, theme_));
            SetBkColor(gr.dc, theme_color(theme_value::window_background, theme_));
        }

        TextOutW(gr.dc, text_left, text_top, caption.c_str(), (int32_t)caption.size());
    }
#endif
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
                active = true;
                redraw();
            break;
            case mouse_event_type::leave:
                active = false;
                redraw();
            break;
            case mouse_event_type::left_up:
                if (click_callback)
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

void Button::SetPosition(const Rect &position_)
{
	auto prevPosition = position;
	position = position_;

	if (parent.lock())
	{
		parent.lock()->Redraw(prevPosition, true);
	}
	
	Redraw();
}

Rect Button::GetPosition() const
{
	return position;
}

void Button::SetParent(std::shared_ptr<Window> window)
{
	parent = window;
}

void Button::ClearParent()
{
	parent.reset();
}

void Button::SetFocus()
{
	if (receiveFocus && enabled && showed)
	{
		focused = true;

		Redraw();
	}
}

bool Button::RemoveFocus()
{
	focused = false;

	Redraw();

	return true;
}

bool Button::Focused() const
{
	return focused;
}

bool Button::Focusing() const
{
	return enabled && showed && receiveFocus;
}

void Button::UpdateTheme(std::shared_ptr<ITheme> theme_)
{
	if (theme && !theme_)
	{
		return;
	}
	theme = theme_;

	if (image)
	{
		image->UpdateTheme(theme);
	}

#ifdef _WIN32
	DestroyPrimitives();
	MakePrimitives();
#endif
}

void Button::Show()
{
	showed = true;
	Redraw();
}

void Button::Hide()
{
	showed = false;
	if (parent.lock())
	{
		parent.lock()->Redraw(position, true);
	}
}

bool Button::Showed() const
{
	return showed;
}

void Button::Enable()
{
	enabled = true;
	Redraw();
}

void Button::Disable()
{
	enabled = false;
	Redraw();
}

bool Button::Enabled() const
{
	return enabled;
}

void Button::SetCaption(const std::wstring &caption_)
{
	caption = caption_;
}

void Button::SetButtonView(ButtonView buttonView_)
{
	buttonView = buttonView_;

#ifdef _WIN32
	DestroyPrimitives();
	MakePrimitives();
#endif

	Redraw();
}

#ifdef _WIN32
void Button::SetImage(int32_t resourceIndex)
{
	if (image)
	{
		image->ChangeImage(resourceIndex);
	}
	else
	{
		image = std::shared_ptr<Image>(new Image(resourceIndex));
	}
	Redraw();
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
    if (parent.lock())
    {
        parent.lock()->redraw(position_);
    }
}

#ifdef _WIN32
void button::make_primitives()
{
    calm_brush = CreateSolidBrush(button_view_ != button_view::image_right_text_no_frame ? theme_color(theme_value::button_calm, theme_) : theme_color(theme_value::window_background, theme_));
    active_brush = CreateSolidBrush(button_view_ != button_view::image_right_text_no_frame ? theme_color(theme_value::button_active, theme_) : theme_color(theme_value::window_background, theme_));
    disabled_brush = CreateSolidBrush(button_view_ != button_view::image_right_text_no_frame ? theme_color(theme_value::button_disabled, theme_) : theme_color(theme_value::window_background, theme_));
    border_pen = CreatePen(PS_SOLID, 1, button_view_ != button_view::image_right_text_no_frame ? theme_color(theme_value::button_border, theme_) : theme_color(theme_value::window_background, theme_));
    focused_border_pen = CreatePen(PS_SOLID, 1, theme_color(theme_value::button_focused_border, theme_));
}

void button::destroy_primitives()
{
    DeleteObject(calm_brush);
    DeleteObject(active_brush);
    DeleteObject(disabled_brush);
    DeleteObject(border_pen);
    DeleteObject(focused_border_pen);
}
#endif

}
