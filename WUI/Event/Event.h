#pragma once

#include <WUI/Event/MouseEvent.h>
#include <WUI/Event/KeyboardEvent.h>
#include <WUI/Event/InternalEvent.h>

namespace WUI
{

enum class EventType
{
	System,
	Mouse,
	Keyboard,
	Internal
};

class IControl;

struct Event
{
	EventType type;
	
	union
	{
		MouseEvent mouseEvent;
		KeyboardEvent keyboardEvent;
		InternalEvent internalEvent;
	};
};

}
