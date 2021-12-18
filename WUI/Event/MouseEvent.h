#pragma once

#include <cstdint>

namespace WUI
{

enum class MouseEventType
{
	Move,
	RightDown,
	RightUp,
	CenterDown,
	CenterUp,
	LeftDown,
	LeftUp,
	RightDouble
};

struct MouseEvent
{
	MouseEventType type;

	int32_t x = 0, y = 0;

	MouseEvent(MouseEventType type_, int32_t x_, int32_t y_)
		: type(type_), x(x_), y(y_)
	{}
};

}
