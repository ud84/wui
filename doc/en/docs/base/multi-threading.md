# Threads and timers

WUI does not promise arbitrary concurrent access to controls. Update an interface
from its owning event thread. Internal locks around subscriptions do not make
`add_control()`, drawing or control state generally thread-safe.

| Platform | `timer` callback |
| --- | --- |
| Windows | Windows timer queue worker |
| Linux | Timer's worker thread |
| macOS | Main UI thread |
| WASM | Browser main thread; background tabs can throttle it |

The actual timer API is `timer(callback)`, `start(interval_ms)` and `stop()`.
Constructing a timer does not start it. Keep it alive while needed. For portable
UI code, send a small notification to the window and update controls in its event
handler, rather than modifying controls directly in a timer/worker callback.

```cpp
#include <wui/system/timer.hpp>

wui::timer timer([weak = std::weak_ptr<wui::window>(window)] {
    if (auto target = weak.lock()) target->emit_event(100, 0);
});
// Handle internal_event_type::user_emitted in the window subscription.
timer.start(100); // milliseconds
// Before captured state or the receiving window is destroyed:
timer.stop();
```

`emit_event(x,y)` delivers an `internal_event_type::user_emitted` event. Transfer
larger data using application-owned storage with appropriate synchronization;
the two integer arguments are not object ownership. Join workers and stop timers
before destroying the data they capture. Do not stop/destroy a Linux worker timer
from its own callback, because `stop()` joins its worker thread.

The WASM backend currently has no WUI pthread/Worker integration. On macOS all UI
initialization, event processing and drawing must run on the main thread.
