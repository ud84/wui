#pragma once

#include <WUI/Control/IControl.h>
#include <WUI/Graphic/Graphic.h>
#include <WUI/Event/Event.h>
#include <WUI/Common/Rect.h>
#include <WUI/Common/Color.h>

#include <string>
#include <functional>
#include <memory>

namespace WUI
{

class Button : public IControl, public std::enable_shared_from_this<Button>
{
public:
	Button(const std::wstring &caption, std::function<void(void)> clickCallback, std::shared_ptr<ITheme> theme = nullptr);
	~Button();

	virtual void Draw(Graphic &gr);
	virtual void ReceiveEvent(const Event &ev);
	
	virtual void SetPosition(const Rect &position);
	virtual Rect GetPosition() const;
	
	virtual void SetParent(std::shared_ptr<Window> window);
	virtual void ClearParent();

	virtual bool SetFocus();
	virtual void RemoveFocus();
	virtual bool Focused();
	
	virtual void UpdateTheme(std::shared_ptr<ITheme> theme = nullptr);

	virtual void Show();
	virtual void Hide();
	virtual bool Showed() const;

	virtual void Enable();
	virtual void Disable();
	virtual bool Enabled() const;

	void SetCaption(const std::wstring &caption);

	void SetCallback(std::function<void(void)> clickCallback);

private:
	std::wstring caption;
	std::function<void(void)> clickCallback;
	std::shared_ptr<ITheme> theme;

	Rect position;

	std::weak_ptr<Window> parent;

	bool showed, enabled, active;

#ifdef _WIN32
	HBRUSH calmBrush, activeBrush, disabledBrush;
	HPEN borderPen;

	void MakePrimitives();
	void DestroyPrimitives();
#endif

	void Redraw();
};

}
