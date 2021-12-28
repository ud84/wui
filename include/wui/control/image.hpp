//
// Copyright (c) 2021-2022 Anton Golovkov (udattsk at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/ud84/WUI
//

#pragma once

#include <wui/control/icontrol.hpp>
#include <wui/graphic/graphic.hpp>
#include <wui/event/event.hpp>
#include <wui/common/rect.hpp>
#include <wui/common/color.hpp>

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
