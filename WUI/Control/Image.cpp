
#include <WUI/Control/Image.h>

#include <WUI/Window/Window.h>

#include <WUI/Theme/Theme.h>

#ifdef _WIN32

#pragma comment( lib, "gdiplus.lib" )

void LoadImageFromResource(WORD imageID, const std::wstring &resourceSection, Gdiplus::Image **pImg)
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

	HGLOBAL hBuffer = ::GlobalAlloc(GMEM_MOVEABLE, imageSize);
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

		::GlobalUnlock(hBuffer);
		::GlobalFree(hBuffer);
		hBuffer = nullptr;
	}
}

void LoadImageFromFile(const std::wstring &fileName, const std::wstring &imagesPath, Gdiplus::Image **pImg)
{
	*pImg = Gdiplus::Image::FromFile(std::wstring(imagesPath + L"\\" + fileName).c_str());
}

void FreeImage(Gdiplus::Image **pImg)
{
	if (*pImg)
	{
		delete *pImg;
		*pImg = nullptr;
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
	fileName(),
	resourceIndex(resourceIndex_),
	img(nullptr)
{
	LoadImageFromResource(resourceIndex, ThemeString(ThemeValue::Images_Path, theme), &img);
}
#endif

Image::Image(const std::wstring &fileName_, std::shared_ptr<ITheme> theme_)
	: theme(theme_),
	position(),
	parent(),
	showed(true),
	fileName(fileName_)
#ifdef _WIN32
	, resourceIndex(0),
	img(nullptr)
#endif
{
	LoadImageFromFile(fileName, ThemeString(ThemeValue::Images_Path, theme), &img);
}

Image::~Image()
{
#ifdef _WIN32
	FreeImage(&img);
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
		Gdiplus::Graphics gr(gr.dc);

		gr.DrawImage(
			img->Clone(),
			Gdiplus::Rect(position.left, position.top, position.width(), position.height()),
			0, 0, img->GetWidth(), img->GetHeight(),
			Gdiplus::UnitPixel,
			nullptr);
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
		parent.lock()->Redraw(prevPosition, true);
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
	else if (!fileName.empty())
	{
		ChangeImage(fileName);
	}
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

	FreeImage(&img);
	LoadImageFromResource(resourceIndex, ThemeString(ThemeValue::Images_Path, theme), &img);
	Redraw();
}
#endif

void Image::ChangeImage(const std::wstring &fileName_)
{
	fileName = fileName_;

	FreeImage(&img);
	LoadImageFromFile(fileName, ThemeString(ThemeValue::Images_Path, theme), &img);
	Redraw();
}

int32_t Image::width() const
{
	if (img)
	{
		return img->GetWidth();
	}
	return 0;
}

int32_t Image::height() const
{
	if (img)
	{
		return img->GetHeight();
	}
	return 0;
}

void Image::Redraw()
{
	if (parent.lock())
	{
		parent.lock()->Redraw(position);
	}
}

}
