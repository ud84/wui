#pragma once

#include <cstdint>

namespace WUI
{

struct Rect
{
	int32_t left, top, right, bottom;

	Rect()
		: left(0), top(0), right(0), bottom(0)
	{}

	//bool operator==()
};

}
