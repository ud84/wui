#pragma once

#include <cstdint>

namespace WUI
{

struct Rect
{
	int32_t left = 0, top = 0, right = 0, bottom = 0;

	inline bool operator==(const Rect &lv)
	{
		return left == lv.left && top == lv.top && right == lv.right && bottom == lv.bottom;
	}

	inline Rect operator+(const Rect &lv)
	{
		return { left + lv.left, top + lv.top, left + lv.right, top + lv.bottom };
	}

	inline bool In(int32_t x, int32_t y)
	{
		return x >= left && x <= right && y >= top && y <= bottom;
	}

	inline int32_t width()
	{
		return right - left;
	}

	inline int32_t height()
	{
		return bottom - top;
	}
};

}
