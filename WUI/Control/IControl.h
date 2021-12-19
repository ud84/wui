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

	virtual void ReceiveEvent(const Event &ev) = 0; /// <- Events from parent window

	virtual void SetPosition(const Rect &position) = 0;
	virtual Rect GetPosition() const = 0;

	virtual void SetParent(Window *window) = 0;

	virtual void UpdateTheme() = 0;

protected:
	~IControl() {}

};

}
