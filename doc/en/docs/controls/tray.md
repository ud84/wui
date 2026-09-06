# Tray Icon

The `tray_icon` class adds an application icon to the system tray.

## Quick Start

```cpp
#include <wui/control/tray_icon.hpp>

auto tray = std::make_shared<wui::tray_icon>(
    window,
    "res/icon.ico",
    "My Application",
    [](wui::tray_icon_action action) {
        if (action == wui::tray_icon_action::left_click) {
            show_window();
        }
    }
);
```

## Actions

```cpp
enum class tray_icon_action {
    left_click, right_click, center_click, message_click
};
```

## API

```cpp
// From file
tray_icon(std::weak_ptr<window> parent,
          std::string_view icon_file_name,
          std::string_view tip,
          std::function<void(tray_icon_action)> click_callback);

// Change icon
void change_icon(std::string_view icon_file_name);

// Change tip
void change_tip(std::string_view tip);

// Show notification
void show_message(std::string_view title, std::string_view message);

// Set callback
void set_callback(std::function<void(tray_icon_action)> cb);
```

## See Also

- [Menu](menu.md) — for context menu
- [Window](../base/interfaces.md#window) — parent window

## Platform behavior

On macOS this uses `NSStatusItem`; `show_message()` shows a popover, not a
Notification Center notification. In WASM no operating-system tray is created.
The Linux branches of this class are currently stubs; no tray icon is created. See the platform guides.

The Windows file constructor expects an ICO file; macOS accepts an image file such as PNG.
