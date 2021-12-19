#pragma once

#include <WUI/Event/MouseEvent.h>
#include <WUI/Event/KeyboardEvent.h>

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
	
	//IControl &author;

	union
	{
		MouseEvent mouseEvent;
		KeyboardEvent keyboardEvent;
	};
};

}
