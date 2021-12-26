
#include <WUI/Control/Image.h>

#include <WUI/Window/Window.h>

#include <WUI/Theme/Theme.h>

#ifdef _WIN32

#pragma comment( lib, "gdiplus.lib" )

void LoadImageFromResource(WORD imageID, const std::wstring &resourceSection, HGLOBAL &hBuffer, Gdiplus::Image **pImg)
{
	HINSTANCE hInst = GetModuleHandle(NULL);
	HRSRC hResource = FindResource(hInst, MAKEINTRESOURCE(imageID), resourceSection.c_str());
	if (!hResource)
		return;

	DWORD imageSize = ::SizeofResource(hInst, hResource);
	if (!imageSize)
		return;

	const void* pResourceData = ::LockResource(::LoadResource(hInst, hResource));
	if (!pResourceData)
		return;

	hBuffer = ::GlobalAlloc(GMEM_MOVEABLE, imageSize);
	if (hBuffer)
	{
		void* pBuffer = ::GlobalLock(hBuffer);
		if (pBuffer)
		{
			CopyMemory(pBuffer, pResourceData, imageSize);

			IStream* pStream = NULL;
			if (::CreateStreamOnHGlobal(hBuffer, FALSE, &pStream) == S_OK)
			{
				*pImg = Gdiplus::Image::FromStream(pStream);
				pStream->Release();
			}
		}
	}
}

void FreeImage(HGLOBAL &hBuffer, Gdiplus::Image **pImg)
{
	if (*pImg)
	{
		delete *pImg;
		*pImg = nullptr;
	}

	if (hBuffer)
	{
		::GlobalUnlock(hBuffer);
		::GlobalFree(hBuffer);
		hBuffer = nullptr;
	}
}

#endif

namespace WUI
{

#ifdef _WIN32
Image::Image(int32_t resourceIndex_, std::shared_ptr<ITheme> theme_)
	: theme(theme_),
	position(),
	parent(),
	showed(true),
	resourceIndex(0),
	imageBuffer(nullptr),
	img(nullptr)
{
	resourceIndex = resourceIndex_;
	LoadImageFromResource(resourceIndex, ThemeString(ThemeValue::Images_Path, theme), imageBuffer, &img);
}
#endif

Image::Image(const std::string &fileName, std::shared_ptr<ITheme> theme_)
	: theme(theme_),
	position(),
	parent(),
	showed(true)
#ifdef _WIN32
	, resourceIndex(0),
	imageBuffer(nullptr),
	img(nullptr)
#endif
{

}

Image::~Image()
{
#ifdef _WIN32
	FreeImage(imageBuffer, &img);
#endif

	if (parent.lock())
	{
		parent.lock()->RemoveControl(shared_from_this());
	}
}

void Image::Draw(Graphic &gr)
{
	if (!showed)
	{
		return;
	}

#ifdef _WIN32
	if (img)
	{
		Gdiplus::ImageAttributes attr;
		attr.SetColorKey(ThemeColor(ThemeValue::Window_Background, theme), ThemeColor(ThemeValue::Window_Background, theme),
			Gdiplus::ColorAdjustTypeBitmap);

		Gdiplus::Graphics gr(gr.dc);

		gr.DrawImage(
			img->Clone(),
			Gdiplus::Rect(position.left, position.top, position.width(), position.height()),
			0, 0, img->GetWidth(), img->GetHeight(),
			Gdiplus::UnitPixel,
			&attr);
	}
#endif
}

void Image::ReceiveEvent(const Event &ev)
{
}

void Image::SetPosition(const Rect &position_)
{
	auto prevPosition = position;
	position = position_;

	if (parent.lock())
	{
		parent.lock()->Redraw(prevPosition);
	}
	
	Redraw();
}

Rect Image::GetPosition() const
{
	return position;
}

void Image::SetParent(std::shared_ptr<Window> window)
{
	parent = window;
}

void Image::ClearParent()
{
	parent.reset();
}

void Image::SetFocus()
{
}

bool Image::RemoveFocus()
{
	return true;
}

bool Image::Focused() const
{
	return false;
}

bool Image::Focusing() const
{
	return false;
}

void Image::UpdateTheme(std::shared_ptr<ITheme> theme_)
{
	if (theme && !theme_)
	{
		return;
	}
	theme = theme_;

	if (resourceIndex)
	{
		ChangeImage(resourceIndex);
	}
	// else if fileName...
}

void Image::Show()
{
	showed = true;
	Redraw();
}

void Image::Hide()
{
	showed = false;
	Redraw();
}

bool Image::Showed() const
{
	return showed;
}

void Image::Enable()
{
}

void Image::Disable()
{
}

bool Image::Enabled() const
{
	return true;
}

#ifdef _WIN32
void Image::ChangeImage(int32_t resourceIndex_)
{
	resourceIndex = resourceIndex_;

	FreeImage(imageBuffer, &img);
	LoadImageFromResource(resourceIndex, ThemeString(ThemeValue::Images_Path, theme), imageBuffer, &img);
	Redraw();
}
#endif

void Image::ChangeImage(const std::string &fileName)
{
	
}

void Image::Redraw()
{
	if (parent.lock())
	{
		parent.lock()->Redraw(position);
	}
}

#ifdef _WIN32

#endif

}
