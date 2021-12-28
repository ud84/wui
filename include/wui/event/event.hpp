//
// Copyright (c) 2021-2022 Anton Golovkov (udattsk at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/ud84/WUI
//

#pragma once

#include <wui/event/mouse_event.hpp>
#include <wui/event/keyboard_event.hpp>
#include <wui/event/internal_event.hpp>

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
