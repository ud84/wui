#pragma once

#include <cstdint>

namespace WUI
{

enum class KeyboardEventType
{
	Down,
	Up
};

struct KeyboardEvent
{
	KeyboardEventType type;

	int32_t key;
};

}
