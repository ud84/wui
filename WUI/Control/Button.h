#pragma once

#include <WUI/Control/IControl.h>
#include <WUI/Graphic/Graphic.h>
#include <WUI/Event/Event.h>
#include <WUI/Common/Rect.h>
#include <WUI/Common/Color.h>

#include <string>
#include <functional>

#ifdef _WIN32

#endif

namespace WUI
{

class Button : public IControl
{
public:
	/// Theme's constants
	const uint32_t Button_Calm_Color   = 10000;
	const uint32_t Button_Active_Color = 10001;
	const uint32_t Button_Border_Color = 10002;
	const uint32_t Button_Text_Color   = 10003;

	Button(const std::string &caption, std::function<void(void)> clickCallback);
	~Button();

	virtual void Draw(Graphic &gr);
	virtual void ReceiveEvent(Event &ev);
	virtual void SetPosition(const Rect &position);
	virtual void SetParent(Window *window);

	void SetCaption(const std::string &caption);

	void SetCallback(std::function<void(void)> clickCallback);

private:
	std::string caption;
	std::function<void(void)> clickCallback;

	Rect position;

	Window *parent;

#ifdef _WIN32
	HBRUSH calmBrush, activeBrush;
	HPEN calmPen, activePen;
	HFONT font;
#endif
};

}
