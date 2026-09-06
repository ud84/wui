# Panel

A panel draws a theme background and optionally custom graphics. It is not a
container or an automatic layout manager. Add related controls to the same window
after the panel. Drawing coordinates belong to the window.

```cpp
#include <wui/control/panel.hpp>

const wui::rect bounds{20, 50, 320, 180};
auto panel = std::make_shared<wui::panel>([bounds](wui::graphic& gr) {
    auto color = wui::make_color(30, 160, 100);
    gr.draw_rect({bounds.left + 20, bounds.top + 20,
                  bounds.left + 60, bounds.top + 60}, color, color, 0, 40);
});
window->add_control(panel, bounds);
```

```cpp
panel(std::string_view theme_control_name = tc, std::shared_ptr<i_theme> theme = nullptr);
panel(std::function<void(graphic&)> draw_callback,
      std::string_view theme_control_name = tc, std::shared_ptr<i_theme> theme = nullptr);
```

For dynamic layouts read current bounds from application state in the callback.
`graphic` has no `draw_circle()` method: a fully rounded square is used above.
The panel theme uses `background`. See [graphics](../base/graphic.md) and [splitter](splitter.md).
