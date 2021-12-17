#pragma once

namespace WUI
{

struct Event;
struct Rect;
struct Graphic;
class Window;

class IControl
{
public:
	virtual void Draw(Graphic &gr) = 0;

	virtual void ReceiveEvent(Event &ev) = 0; /// <- Events from parent window

	virtual void SetPosition(const Rect &rect) = 0;

	virtual void SetParent(Window &window) = 0;

protected:
	~IControl() {}

};

}
