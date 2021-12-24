#pragma once

#include <cstdint>

namespace WUI
{

enum class InternalEventType
{
	SetFocus,
	RemoveFocus
};

struct InternalEvent
{
	InternalEventType type;
};

}
