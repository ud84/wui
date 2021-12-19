
#include <WUI/Control/Button.h>

#include <WUI/Window/Window.h>

#include <WUI/Theme/Theme.h>

namespace WUI
{

Button::Button(const std::string &caption_, std::function<void(void)> clickCallback_)
	: caption(caption_),
	clickCallback(clickCallback_),
	position(),
	parent(nullptr),
	active(false)
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

	if (parent)
	{
		parent->RemoveControl(*this);
	}
}

void Button::Draw(Graphic &gr)
{
#ifdef _WIN32
	SelectObject(gr.dc, active ? activePen : calmPen);
	SelectObject(gr.dc, active ? activeBrush : calmBrush);

	RoundRect(gr.dc, position.left, position.top, position.right, position.bottom, 5, 5);

	SetTextColor(gr.dc, ThemeColor(ThemeValue::Button_Text));
	SetBkColor(gr.dc, active ? ThemeColor(ThemeValue::Button_Active) : ThemeColor(ThemeValue::Button_Calm));

	RECT textRect = { 0 };
	DrawTextA(gr.dc, caption.c_str(), static_cast<int32_t>(caption.size()), &textRect, DT_CALCRECT);
	
	auto top = position.top + ((position.bottom - position.top - textRect.bottom) / 2);
	auto left = position.left + ((position.right - position.left - textRect.right) / 2);
	TextOutA(gr.dc, left, top, caption.c_str(), (int32_t)caption.size());
#endif
}

void Button::ReceiveEvent(const Event &ev)
{
	if (ev.type == EventType::Mouse)
	{
		switch (ev.mouseEvent.type)
		{
			case MouseEventType::Enter:
				active = true;
				parent->Redraw(position);
			break;
			case MouseEventType::Leave:
				active = false;
				parent->Redraw(position);
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

void Button::SetParent(Window *window)
{
	parent = window;
}

void Button::UpdateTheme()
{
#ifdef _WIN32
	DestroyPrimitives();
	MakePrimitives();
#endif
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
