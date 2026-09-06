# Main application loop

Call `framework::init()` before creating GUI objects, initialize a window, then
call `framework::run()`. Call `framework::stop()` when your application is finished.
`started()` reports an active framework instance; `get_error()` exposes its error.
There is no `framework::end()` API.

| Backend | Event processing |
| --- | --- |
| Windows | Win32 message loop |
| Linux | XCB event processing for windows; framework waits until stopped |
| macOS | AppKit main-thread event loop |
| WASM | Asyncify loop that yields to the browser and returns after `stop()` |

Keep application objects alive across `run()`. Closing a window and stopping the
application are separate operations: explicitly stop in the main close callback
when that is the behavior you want. A scratchpad can close without stopping its owner.

```cpp
window->init("Application", {-1, -1, 800, 600}, wui::window_style::frame,
    [] { wui::framework::stop(); });
wui::framework::run();
```

See [Hello world](../howto/hello-world.md) and [threading](multi-threading.md).
