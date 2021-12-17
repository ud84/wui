
#include <WUI/Control/Button.h>

namespace WUI
{

Button::Button()
	: clickCallback()
{
}

Button::~Button()
{
}

void Button::Draw(Graphic &gr)
{
}

void Button::ReceiveEvent(Event &ev)
{
}

void Button::SetPosition(const Rect &rect)
{
}

void Button::SetParent(Window &window)
{
}

void Button::SetCaption(const std::string &caption)
{
}

void Button::SetCallback(std::function<void(void)> clickCallback_)
{
	clickCallback = clickCallback_;
}

}
