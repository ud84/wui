#pragma once

#ifdef _WIN32
#include <windows.h>
#endif

namespace WUI
{

struct Graphic
{
#ifdef _WIN32
	HDC dc;
#endif
};

}
