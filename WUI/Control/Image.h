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
#include <gdiplus.h>
#endif

namespace WUI
{

class Image : public IControl, public std::enable_shared_from_this<Image>
{
public:
#ifdef _WIN32
	Image(int32_t resourceIndex, std::shared_ptr<ITheme> theme = nullptr);
#endif
	Image(const std::wstring &fileName, std::shared_ptr<ITheme> theme = nullptr);
	~Image();

	virtual void Draw(Graphic &gr);
	virtual void ReceiveEvent(const Event &ev);
	
	virtual void SetPosition(const Rect &position);
	virtual Rect GetPosition() const;
	
	virtual void SetParent(std::shared_ptr<Window> window);
	virtual void ClearParent();

	virtual void SetFocus();
	virtual bool RemoveFocus();
	virtual bool Focused() const;
	virtual bool Focusing() const;
	
	virtual void UpdateTheme(std::shared_ptr<ITheme> theme = nullptr);

	virtual void Show();
	virtual void Hide();
	virtual bool Showed() const;

	virtual void Enable();
	virtual void Disable();
	virtual bool Enabled() const;

#ifdef _WIN32
	void ChangeImage(int32_t resourceIndex);
#endif
	void ChangeImage(const std::wstring &fileName);

	int32_t width() const;
	int32_t height() const;

private:
	std::shared_ptr<ITheme> theme;

	Rect position;

	std::weak_ptr<Window> parent;

	bool showed;

	std::wstring fileName;
	
#ifdef _WIN32
	int32_t resourceIndex;
	Gdiplus::Image *img;
#endif

	void Redraw();
};

}
