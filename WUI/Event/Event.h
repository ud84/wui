#pragma once

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
	
	IControl &author;

	void *data;

	Event(EventType type_, IControl &author_, void *data_)
		: type(type_), author(author_), data(data_)
	{}
};

}
