#pragma once

#include <WUI/Control/IControl.h>
#include <WUI/Graphic/Graphic.h>
#include <WUI/Event/Event.h>
#include <WUI/Common/Rect.h>
#include <WUI/Common/Color.h>

#include <string>
#include <functional>
#include <memory>

#ifdef _WIN32

#endif

namespace WUI
{

class Button : public IControl, public std::enable_shared_from_this<Button>
{
public:
	Button(const std::string &caption, std::function<void(void)> clickCallback);
	~Button();

	virtual void Draw(Graphic &gr);
	virtual void ReceiveEvent(const Event &ev);
	
	virtual void SetPosition(const Rect &position);
	virtual Rect GetPosition() const;
	
	virtual void SetParent(std::shared_ptr<Window> window);
	virtual void ClearParent();
	
	virtual void UpdateTheme();

	virtual void Show();
	virtual void Hide();
	virtual bool Showed() const;

	virtual void Enable();
	virtual void Disable();
	virtual bool Enabled() const;

	void SetCaption(const std::string &caption);

	void SetCallback(std::function<void(void)> clickCallback);

private:
	std::string caption;
	std::function<void(void)> clickCallback;

	Rect position;

	std::weak_ptr<Window> parent;

	bool showed, enabled, active;

#ifdef _WIN32
	HBRUSH calmBrush, activeBrush, disabledBrush;
	HPEN borderPen;

	void MakePrimitives();
	void DestroyPrimitives();
#endif
};

}
