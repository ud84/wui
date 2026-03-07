# Panel

The `panel` control is a container for grouping elements or creating custom drawing areas.

## Quick Start

```cpp
#include <wui/control/panel.hpp>

// Simple panel
auto panel = std::make_shared<wui::panel>();
window->add_control(panel, {10, 10, 200, 100});

// Panel with drawing callback
auto custom_panel = std::make_shared<wui::panel>(
    [](wui::graphic& gr) {
        // Custom drawing
        gr.draw_rect({0, 0, 100, 50}, wui::make_color(255, 0, 0));
    }
);
window->add_control(custom_panel, {10, 120, 200, 200});
```

## API

![WUI Panel](../img/panel.png)

### Constructors

```cpp
// Empty panel
panel(std::string_view theme_control_name = tc, 
      std::shared_ptr<i_theme> theme_ = nullptr);

// Panel with drawing callback
panel(std::function<void(wui::graphic&)> draw_callback, 
      std::string_view theme_control_name = tc, 
      std::shared_ptr<i_theme> theme_ = nullptr);
```

## Examples

### Grouping Panel

```cpp
// Panel for grouping form elements
auto form_panel = std::make_shared<wui::panel>();
form_panel->set_position({10, 10, 300, 200});
window->add_control(form_panel);

// Add elements to panel
auto name_label = std::make_shared<wui::text>("Name:");
auto name_input = std::make_shared<wui::input>();
window->add_control(name_label, {20, 20, 60, 40});
window->add_control(name_input, {70, 20, 200, 40});
```

### Custom Drawing

```cpp
// Panel with gradient
auto gradient_panel = std::make_shared<wui::panel>(
    [](wui::graphic& gr) {
        // Draw gradient
        for (int y = 0; y < 100; y++) {
            auto color = wui::make_color(100 + y, 150, 200);
            gr.draw_rect({0, y, 200, y + 1}, color);
        }
    }
);
window->add_control(gradient_panel, {10, 10, 200, 100});
```

### Status Indicator

```cpp
auto status_panel = std::make_shared<wui::panel>(
    [is_online](wui::graphic& gr) {
        auto color = is_online ? 
            wui::make_color(0, 200, 0) :   // Green
            wui::make_color(200, 0, 0);    // Red
        
        gr.draw_circle({25, 25, 50, 50}, color, true);
    }
);
window->add_control(status_panel, {10, 10, 50, 50});
```

## Theming

```json
{
  "type": "panel",
  "background": "#f0f0f0"
}
```

## See Also

- [Graphic](../base/graphic.md) — for working with graphic
- [Visual Themes](../base/theme.md)
