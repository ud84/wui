
#include <WUI/Control/Button.h>

#include <WUI/Window/Window.h>

#include <WUI/Theme/Theme.h>

namespace WUI
{

Button::Button(const std::string &caption_, std::function<void(void)> clickCallback_)
	: caption(caption_),
	clickCallback(clickCallback_),
	position(),
	parent(),
	showed(true), active(false)
#ifdef _WIN32
	,calmBrush(0), activeBrush(0),
	calmPen(0), activePen(0)
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
	DrawTextA(gr.dc, caption.c_str(), static_cast<int32_t>(caption.size()), &textRect, DT_CALCRECT);

	if (textRect.right > position.width())
	{
		position.right = position.left + textRect.right + 10;
	}

	SetTextColor(gr.dc, ThemeColor(ThemeValue::Button_Text));
	SetBkColor(gr.dc, active ? ThemeColor(ThemeValue::Button_Active) : ThemeColor(ThemeValue::Button_Calm));

	SelectObject(gr.dc, active ? activePen : calmPen);
	SelectObject(gr.dc, active ? activeBrush : calmBrush);

	RoundRect(gr.dc, position.left, position.top, position.right, position.bottom, 5, 5);
	
	auto top = position.top + ((position.bottom - position.top - textRect.bottom) / 2);
	auto left = position.left + ((position.right - position.left - textRect.right) / 2);
	TextOutA(gr.dc, left, top, caption.c_str(), (int32_t)caption.size());
#endif
}

void Button::ReceiveEvent(const Event &ev)
{
	if (ev.type == EventType::Mouse && parent.lock() && showed)
	{
		switch (ev.mouseEvent.type)
		{
			case MouseEventType::Enter:
				active = true;
				parent.lock()->Redraw(position);
			break;
			case MouseEventType::Leave:
				active = false;
				parent.lock()->Redraw(position);
			break;
			case MouseEventType::LeftUp:
				clickCallback();
			break;
		}
	}
}

void Button::SetPosition(const Rect &position_)
{
	position = position_;
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

void Button::UpdateTheme()
{
#ifdef _WIN32
	DestroyPrimitives();
	MakePrimitives();
#endif
}

void Button::Show()
{
	showed = true;
	if (parent.lock())
	{
		parent.lock()->Redraw(position);
	}
}

void Button::Hide()
{
	showed = false;
	if (parent.lock())
	{
		parent.lock()->Redraw(position);
	}
}

void Button::SetCaption(const std::string &caption_)
{
	caption = caption_;
}

void Button::SetCallback(std::function<void(void)> clickCallback_)
{
	clickCallback = clickCallback_;
}

#ifdef _WIN32
void Button::MakePrimitives()
{
	calmBrush = CreateSolidBrush(ThemeColor(ThemeValue::Button_Calm));
	activeBrush = CreateSolidBrush(ThemeColor(ThemeValue::Button_Active));
	calmPen = CreatePen(PS_SOLID, 1, ThemeColor(ThemeValue::Button_Calm));
	activePen = CreatePen(PS_SOLID, 1, ThemeColor(ThemeValue::Button_Active));
}

void Button::DestroyPrimitives()
{
	DeleteObject(calmBrush);
	DeleteObject(activeBrush);
	DeleteObject(calmPen);
	DeleteObject(activePen);
}
#endif

}
