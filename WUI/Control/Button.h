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
	Button(const std::string &caption, std::function<void(void)> clickCallback);
	~Button();

	virtual void Draw(Graphic &gr);
	virtual void ReceiveEvent(Event &ev);
	virtual void SetPosition(const Rect &position);
	virtual void SetParent(Window *window);
	virtual void UpdateTheme();

	void SetCaption(const std::string &caption);

	void SetCallback(std::function<void(void)> clickCallback);

private:
	std::string caption;
	std::function<void(void)> clickCallback;

	Rect position;

	Window *parent;

	bool active;

#ifdef _WIN32
	HBRUSH calmBrush, activeBrush;
	HPEN calmPen, activePen;
	HFONT font;

	void MakePrimitives();
	void DestroyPrimitives();
#endif
};

}
