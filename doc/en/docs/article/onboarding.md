# Application structure

WUI provides shared C++ controls and a platform window/graphics backend. A useful
starting point is [Hello world](../howto/hello-world.md); a larger example is
`examples/demo`, the [live Showcase](https://libwui.org/wasm/demo/).

## Lifecycle

1. Initialize `framework` and load a complete theme and locale.
2. Create a `shared_ptr<window>` and controls; add them with explicit bounds.
3. Register callbacks, initialize the native/browser window, then call `framework::run()`.
4. Call `framework::stop()` when the application should exit.

## Geometry and ownership

Control bounds are `{left, top, right, bottom}`, not x/y/width/height. Recompute
positions on size events; a `panel` draws a background but does not own or lay out
other controls. Add controls to their window, in drawing order.

A window retains its controls; controls hold a weak reference to the parent.
Avoid a control capturing its own `shared_ptr` in a callback: use a weak capture.
Keep stack references alive until callbacks are removed. See [ownership](../base/ownership.md).

## Interaction

Use button/input/select callbacks for local actions and window subscriptions for
broader events. `subscribe()` returns an ID for `unsubscribe()`. A checkbox changes
state before its callback; a menu callback currently receives a visible row index,
not the item's stable ID. See [events](../base/event.md) and [controls](../controls/all.md).

## Themes, localization and background work

Theme fonts are JSON objects. Loading a theme is followed by `window->update_theme()`;
loading a locale is followed by updating application captions explicitly.
Use [resources](../base/resources.md), [themes](../base/theme.md) and [locales](../base/locale.md).

Keep UI updates on the owning event thread. Timers have different callback threads
on different platforms; use `window::emit_event()` to marshal background results.
Read [threading and timers](../base/multi-threading.md) before adding a worker.

## Platform checks

[macOS](../howto/macos.md) and [WASM](../howto/wasm.md) share controls but have their own
resource, clipboard, persistence and system integration behavior. Browser examples
are single-threaded and use an in-memory filesystem. Native dialogs and system
tray behavior should not be assumed identical on every platform.
