
#include <WUI/Control/Button.h>

#include <WUI/Window/Window.h>

#include <WUI/Theme/Theme.h>

namespace WUI
{

Button::Button(const std::string &caption_, std::function<void(void)> clickCallback_)
	: caption(caption_),
	clickCallback(clickCallback_),
	position(),
	parent(nullptr)
#ifdef _WIN32
	,calmBrush(CreateSolidBrush(ThemeColor(Button_Calm_Color))), activeBrush(0),
	calmPen(0), activePen(0),
	font(0)
#endif
{
}

Button::~Button()
{
	if (parent)
	{
		parent->RemoveControl(*this);
	}
}

void Button::Draw(Graphic &gr)
{
}

void Button::ReceiveEvent(Event &ev)
{
}

void Button::SetPosition(const Rect &position_)
{
	position = position_;
}

void Button::SetParent(Window *window)
{
	parent = window;
}

void Button::SetCaption(const std::string &caption_)
{
	caption = caption_;
}

void Button::SetCallback(std::function<void(void)> clickCallback_)
{
	clickCallback = clickCallback_;
}

}
