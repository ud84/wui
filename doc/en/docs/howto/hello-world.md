# Hello world

Start from `examples/hello_world` for a complete application with themes, locales,
configuration and platform resource packaging. Build `hello_world` using the
[installation guide](setup.md), or [run it in the browser](https://libwui.org/wasm/hello_world/).

## A minimal C++ example

The following example uses the complete `res/dark.json` from an existing example.
Keep its associated resources. Use this `main()` for macOS, Linux or WASM; a
Windows GUI subsystem executable uses the WinMain entry point from the bundled example.

```cpp
#include <wui/framework/framework.hpp>
#include <wui/window/window.hpp>
#include <wui/control/text.hpp>
#include <wui/control/button.hpp>
#include <wui/theme/theme.hpp>
#include <memory>

int main()
{
    wui::framework::init();
    if (!wui::set_default_theme_from_file("dark", "res/dark.json")) return 1;
    auto window = std::make_shared<wui::window>();
    auto label = std::make_shared<wui::text>("Hello, WUI!",
        wui::hori_alignment::center, wui::vert_alignment::center);
    auto button = std::make_shared<wui::button>("Say hello",
        [label] { label->set_text("Hello from shared C++ controls!"); });
    window->add_control(label, {20, 50, 400, 110});
    window->add_control(button, {110, 130, 310, 170});
    if (!window->init("Hello WUI", {-1, -1, 420, 220}, wui::window_style::frame,
        [] { wui::framework::stop(); })) return 1;
    wui::framework::run();
}
```

Controls use `{left, top, right, bottom}`. Window `init()` accepts
`{-1, -1, width, height}` as a special request to center the window.

Keep window, dialog and callback state alive while `framework::run()` is active.
The close callback calls `framework::stop()`; creating a window does not replace
running the event loop. The WASM backend uses Asyncify to preserve this lifetime.

The complete example adds theme/language switching, INI/registry configuration,
Unicode name input and confirmation dialogs. Its source files are `hw.cpp`,
`MainFrame/MainFrame.h`, `MainFrame/MainFrame.cpp` and `Resource.h`.

On macOS resources live in `Contents/Resources/res`; on Linux beside the executable;
on Windows in `.rc` resources; in WASM under virtual `/res`. See [resources](../base/resources.md).
