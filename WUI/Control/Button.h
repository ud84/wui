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

class Image;

enum class ButtonView
{
	OnlyText,
	OnlyImage,
	ImageRightText,
	ImageBottomText
};

class Button : public IControl, public std::enable_shared_from_this<Button>
{
public:
	Button(const std::wstring &caption, std::function<void(void)> clickCallback, std::shared_ptr<ITheme> theme = nullptr);

#ifdef _WIN32
	Button(const std::wstring &caption, std::function<void(void)> clickCallback, ButtonView buttonView, int32_t imageResourceIndex, int32_t imageSize, std::shared_ptr<ITheme> theme = nullptr);
#endif
	Button(const std::wstring &caption, std::function<void(void)> clickCallback, ButtonView buttonView, const std::wstring &imageFileName, int32_t imageSize, std::shared_ptr<ITheme> theme = nullptr);
	~Button();

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

	void SetCaption(const std::wstring &caption);

	void SetButtonView(ButtonView buttonView);
#ifdef _WIN32
	void SetImage(int32_t resourceIndex);
#endif
	void SetImage(const std::wstring &fileName);

	void EnableReceiveFocus();
	void DisableReceiveFocus();

	void SetCallback(std::function<void(void)> clickCallback);

private:
	ButtonView buttonView;
	std::wstring caption;
	std::shared_ptr<Image> image;
	int32_t imageSize;
	std::function<void(void)> clickCallback;
	std::shared_ptr<ITheme> theme;

	Rect position;

	std::weak_ptr<Window> parent;

	bool showed, enabled;
	bool active, focused;
	bool receiveFocus;

#ifdef _WIN32
	HBRUSH calmBrush, activeBrush, disabledBrush;
	HPEN borderPen, focusedBorderPen;

	void MakePrimitives();
	void DestroyPrimitives();
#endif

	void Redraw();
};

}
