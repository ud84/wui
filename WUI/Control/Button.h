#pragma once

#include <WUI/Control/IControl.h>
#include <WUI/Graphic/Graphic.h>
#include <WUI/Event/Event.h>
#include <WUI/Common/Rect.h>

#include <string>
#include <functional>

namespace WUI
{

class Button : public IControl
{
public:
	Button();
	~Button();

	virtual void Draw(Graphic &gr);
	virtual void ReceiveEvent(Event &ev);
	virtual void SetPosition(const Rect &rect);
	virtual void SetParent(Window &window);

	void SetCaption(const std::string &caption);

	void SetCallback(std::function<void(void)> clickCallback);

private:
	std::function<void(void)> clickCallback;
};

}
