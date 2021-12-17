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

	Rect(int32_t left_, int32_t top_, int32_t right_, int32_t bottom_)
		: left(left_), top(top_), right(right_), bottom(bottom_)
	{}

	//bool operator==()
};

}
