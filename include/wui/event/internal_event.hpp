#pragma once

namespace WUI
{

enum class InternalEventType
{
	ExecuteFocused
};

struct InternalEvent
{
	InternalEventType type;
};

}
