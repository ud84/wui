#pragma once

namespace WUI
{

typedef unsigned long Color;

static inline Color MakeColor(unsigned char red, unsigned char green, unsigned char blue)
{
	return ((red) | (static_cast<unsigned short>(green) << 8)) | (static_cast<unsigned long>(blue) << 16);
}

}
