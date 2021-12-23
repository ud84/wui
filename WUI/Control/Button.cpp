
#include <WUI/Control/Button.h>

#include <WUI/Window/Window.h>

#include <WUI/Theme/Theme.h>

namespace WUI
{

Button::Button(const std::wstring &caption_, std::function<void(void)> clickCallback_, std::shared_ptr<ITheme> theme_)
	: caption(caption_),
	clickCallback(clickCallback_),
	theme(theme_),
	position(),
	parent(),
	showed(true), enabled(true), active(false)
#ifdef _WIN32
	,calmBrush(0), activeBrush(0), disabledBrush(0),
	borderPen(0)
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

	if (textRect.right > position.width())
	{
		position.right = position.left + textRect.right + 10;
	}

	SetTextColor(gr.dc, ThemeColor(ThemeValue::Button_Text, theme));
	SetBkColor(gr.dc, enabled ? (active ? ThemeColor(ThemeValue::Button_Active, theme) : ThemeColor(ThemeValue::Button_Calm, theme)) : ThemeColor(ThemeValue::Button_Disabled, theme));

	SelectObject(gr.dc, borderPen);
	SelectObject(gr.dc, enabled ? (active ? activeBrush : calmBrush) : disabledBrush);

	auto rnd = ThemeDimension(ThemeValue::Button_Round, theme);

	RoundRect(gr.dc, position.left, position.top, position.right, position.bottom, rnd, rnd);
	
	auto top = position.top + ((position.bottom - position.top - textRect.bottom) / 2);
	auto left = position.left + ((position.right - position.left - textRect.right) / 2);
	TextOutW(gr.dc, left, top, caption.c_str(), (int32_t)caption.size());
#endif
}

void Button::ReceiveEvent(const Event &ev)
{
	if (ev.type == EventType::Mouse && parent.lock() && showed && enabled)
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
}

void Button::SetPosition(const Rect &position_)
{
	auto prevPosition = position;
	position = position_;

	if (parent.lock())
	{
		parent.lock()->Redraw(prevPosition);
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

void Button::UpdateTheme(std::shared_ptr<ITheme> theme_)
{
	if (theme && !theme_)
	{
		return;
	}
	theme = theme_;

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
	Redraw();
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
}

void Button::DestroyPrimitives()
{
	DeleteObject(calmBrush);
	DeleteObject(activeBrush);
	DeleteObject(disabledBrush);
	DeleteObject(borderPen);
}
#endif

}
