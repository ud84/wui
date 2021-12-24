#pragma once

#include <memory>

namespace WUI
{

struct Event;
struct Rect;
struct Graphic;
class Window;
class ITheme;

class IControl
{
public:
	virtual void Draw(Graphic &gr) = 0;

	virtual void ReceiveEvent(const Event &ev) = 0; /// Events from parent window

	virtual void SetPosition(const Rect &position) = 0;
	virtual Rect GetPosition() const = 0;

	virtual void SetParent(std::shared_ptr<Window> window) = 0;
	virtual void ClearParent() = 0;

	virtual bool SetFocus() = 0; /// Returns false if the control does not support focusing
	virtual bool RemoveFocus() = 0; /// Returns false if the control changes focus within its internal controls
	virtual bool Focused() const = 0; /// Returns true if the control is focused
	virtual bool Focusing() const = 0; /// Returns true if the control receives focus

	virtual void UpdateTheme(std::shared_ptr<ITheme> theme = nullptr) = 0;

	virtual void Show() = 0;
	virtual void Hide() = 0;
	virtual bool Showed() const = 0;

	virtual void Enable() = 0;
	virtual void Disable() = 0;
	virtual bool Enabled() const = 0;

protected:
	~IControl() {}

};

}
