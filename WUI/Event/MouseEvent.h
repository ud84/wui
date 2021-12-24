#pragma once

#include <cstdint>

namespace WUI
{

enum class MouseEventType
{
	Move,
	Enter,
	Leave,
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

	int32_t x, y;
};

}
