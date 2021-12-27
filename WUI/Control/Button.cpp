
#include <WUI/Control/Button.h>

#include <WUI/Window/Window.h>

#include <WUI/Control/Image.h>

#include <WUI/Theme/Theme.h>

namespace WUI
{

Button::Button(const std::wstring &caption_, std::function<void(void)> clickCallback_, std::shared_ptr<ITheme> theme_)
	: buttonView(ButtonView::OnlyText),
	caption(caption_),
	image(),
	imageSize(0),
	clickCallback(clickCallback_),
	theme(theme_),
	position(),
	parent(),
	showed(true), enabled(true), active(false), focused(false),
	receiveFocus(true)
#ifdef _WIN32
	, calmBrush(0), activeBrush(0), disabledBrush(0),
	borderPen(0), focusedBorderPen(0)
#endif
{
#ifdef _WIN32
	MakePrimitives();
#endif
}

#ifdef _WIN32
Button::Button(const std::wstring &caption_, std::function<void(void)> clickCallback_, ButtonView buttonView_, int32_t imageResourceIndex_, int32_t imageSize_, std::shared_ptr<ITheme> theme_)
	: buttonView(buttonView_),
	caption(caption_),
	image(new Image(imageResourceIndex_, theme_)),
	imageSize(imageSize_),
	clickCallback(clickCallback_),
	theme(theme_),
	position(),
	parent(),
	showed(true), enabled(true), active(false), focused(false),
	receiveFocus(true),
	calmBrush(0), activeBrush(0), disabledBrush(0),
	borderPen(0), focusedBorderPen(0)
{
	MakePrimitives();
}
#endif

Button::Button(const std::wstring &caption_, std::function<void(void)> clickCallback_, ButtonView buttonView_, const std::wstring &imageFileName_, int32_t imageSize_, std::shared_ptr<ITheme> theme_)
	: buttonView(buttonView_),
	caption(caption_),
	image(new Image(imageFileName_, theme_)),
	imageSize(imageSize_),
	clickCallback(clickCallback_),
	theme(theme_),
	position(),
	parent(),
	showed(true), enabled(true), active(false), focused(false),
	receiveFocus(true)
#ifdef _WIN32
	, calmBrush(0), activeBrush(0), disabledBrush(0),
	borderPen(0), focusedBorderPen(0)
#endif
{
#ifdef _WIN32
	MakePrimitives();
#endif
}

Button::~Button()
{
#ifdef _WIN32
	DestroyPrimitives();
#endif

	if (parent.lock())
	{
		parent.lock()->RemoveControl(shared_from_this());
	}
}

void Button::Draw(Graphic &gr)
{
	if (!showed)
	{
		return;
	}

#ifdef _WIN32
	RECT textRect = { 0 };
	DrawTextW(gr.dc, caption.c_str(), static_cast<int32_t>(caption.size()), &textRect, DT_CALCRECT);

	int32_t textTop = 0, textLeft = 0, imageLeft = 0, imageTop = 0;

	switch (buttonView)
	{
		case ButtonView::OnlyText:
			if (textRect.right + 10 > position.width())
			{
				position.right = position.left + textRect.right + 10;
			}

			textTop = position.top + ((position.bottom - position.top - textRect.bottom) / 2);
			textLeft = position.left + ((position.right - position.left - textRect.right) / 2);
		break;
		case ButtonView::OnlyImage:
			if (image)
			{
				if (imageSize + 10 > position.width())
				{
					position.right = position.left + imageSize + 10;
				}
				if (imageSize + 10 > position.height())
				{
					position.bottom = position.top + imageSize + 10;
				}

				imageTop = position.top + ((position.bottom - position.top - imageSize) / 2);
				imageLeft = position.left + ((position.right - position.left - imageSize) / 2);
			}
		break;
		case ButtonView::ImageRightText:
			if (image)
			{
				if (imageSize + textRect.right + 10 > position.width())
				{
					position.right = position.left + textRect.right + imageSize + 10;
				}
				if (imageSize + 10 > position.height())
				{
					position.bottom = position.top + imageSize + 10;
				}

				textTop = position.top + ((position.bottom - position.top - textRect.bottom) / 2);
				imageLeft = position.left + ((position.right - position.left - textRect.right - imageSize - 5) / 2);
				imageTop = position.top + ((position.bottom - position.top - imageSize) / 2);
				textLeft = imageLeft + imageSize + 5;
			}
		break;
		case ButtonView::ImageBottomText:
			if (image)
			{
				if (imageSize + 10 > position.width())
				{
					position.right = position.left + imageSize + 10;
				}
				if (imageSize + textRect.bottom + 10 > position.height())
				{
					position.bottom = position.top + textRect.bottom + imageSize + 10;
				}

				imageTop = position.top + ((position.bottom - position.top - textRect.bottom - imageSize - 5) / 2);
				textTop = imageTop + imageSize + 5;
				textLeft = position.left + ((position.right - position.left - textRect.right) / 2);
				imageLeft = position.left + ((position.right - position.left - imageSize) / 2);
			}
		break;
	}

	SetTextColor(gr.dc, ThemeColor(ThemeValue::Button_Text, theme));
	SetBkColor(gr.dc, enabled ? (active ? ThemeColor(ThemeValue::Button_Active, theme) : ThemeColor(ThemeValue::Button_Calm, theme)) : ThemeColor(ThemeValue::Button_Disabled, theme));

	SelectObject(gr.dc, !focused ? borderPen : focusedBorderPen);
	SelectObject(gr.dc, enabled ? (active ? activeBrush : calmBrush) : disabledBrush);

	auto rnd = ThemeDimension(ThemeValue::Button_Round, theme);

	RoundRect(gr.dc, position.left, position.top, position.right, position.bottom, rnd, rnd);
	
	if (buttonView != ButtonView::OnlyText)
	{
		image->SetPosition( { imageLeft, imageTop, imageLeft + imageSize, imageTop + imageSize } );
		image->Draw(gr);
	}

	if (buttonView != ButtonView::OnlyImage)
	{
		TextOutW(gr.dc, textLeft, textTop, caption.c_str(), (int32_t)caption.size());
	}
#endif
}

void Button::ReceiveEvent(const Event &ev)
{
	if (!showed || !enabled)
	{
		return;
	}

	if (ev.type == EventType::Mouse)
	{
		switch (ev.mouseEvent.type)
		{
			case MouseEventType::Enter:
				active = true;
				Redraw();
			break;
			case MouseEventType::Leave:
				active = false;
				Redraw();
			break;
			case MouseEventType::LeftUp:
				if (clickCallback)
				{
					clickCallback();
				}
			break;
		}
	}
	else if (ev.type == EventType::Internal)
	{
		if (ev.internalEvent.type == InternalEventType::ExecuteFocused && clickCallback)
		{
			clickCallback();
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

void Button::SetImage(const std::wstring &fileName)
{
	if (image)
	{
		image->ChangeImage(fileName);
	}
	else
	{
		image = std::shared_ptr<Image>(new Image(fileName));
	}
	Redraw();
}

void Button::EnableReceiveFocus()
{
	receiveFocus = true;
}

void Button::DisableReceiveFocus()
{
	receiveFocus = false;
}

void Button::SetCallback(std::function<void(void)> clickCallback_)
{
	clickCallback = clickCallback_;
}

void Button::Redraw()
{
	if (parent.lock())
	{
		parent.lock()->Redraw(position);
	}
}

#ifdef _WIN32
void Button::MakePrimitives()
{
	calmBrush = CreateSolidBrush(ThemeColor(ThemeValue::Button_Calm, theme));
	activeBrush = CreateSolidBrush(ThemeColor(ThemeValue::Button_Active, theme));
	disabledBrush = CreateSolidBrush(ThemeColor(ThemeValue::Button_Disabled, theme));
	borderPen = CreatePen(PS_SOLID, 1, ThemeColor(ThemeValue::Button_Border, theme));
	focusedBorderPen = CreatePen(PS_SOLID, 1, ThemeColor(ThemeValue::Button_FocusedBorder, theme));
}

void Button::DestroyPrimitives()
{
	DeleteObject(calmBrush);
	DeleteObject(activeBrush);
	DeleteObject(disabledBrush);
	DeleteObject(borderPen);
	DeleteObject(focusedBorderPen);
}
#endif

}
