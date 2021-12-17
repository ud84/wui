
#include <WUI/Control/Button.h>

#include <WUI/Window/Window.h>

namespace WUI
{

Button::Button()
	: caption(),
	clickCallback(),
	parent(nullptr)
{
}

Button::Button(const std::string &caption_, std::function<void(void)> clickCallback_)
	: caption(caption_),
	clickCallback(clickCallback_),
	parent(nullptr)
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

void Button::SetPosition(const Rect &rect)
{
}

void Button::SetParent(Window &window)
{
	parent = &window;
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
