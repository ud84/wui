# All Controls

WUI library provides a set of standard controls for creating user interfaces.

## Standard Controls

| Control | Description |
|---------|-------------|
| [Button](button.md) | Button for user interaction. Supports text, images, toggles |
| [Image](image.md) | Image display with visual themes support |
| [Text](text.md) | Non-editable text and alignment |
| [Scroll](scroll.md) | Standalone scrollbar |
| [Input](input.md) | Text input field. Single-line, multi-line, password, read-only |
| [List](list.md) | List of items with columns, scrolling, and custom drawing |
| [Menu](menu.md) | Context menu with nested items and icons support |
| [Message](message.md) | Modal dialogs for displaying messages |
| [Panel](panel.md) | Background and custom drawing; not a container |
| [Progress](progress.md) | Progress indicator (horizontal/vertical) |
| [Select](select.md) | Dropdown list for selecting one value |
| [Slider](slider.md) | Slider for selecting a value from a range |
| [Splitter](splitter.md) | Draggable divider for resizing areas |
| [Tooltip](tooltip.md) | Popup hints for interface elements |
| [Tray](tray.md) | System tray icon with notifications |

## Base Interface

Visual controls implement `i_control`. `message` and `tray_icon` are helper classes with separate APIs:

```cpp
class i_control
{
public:
    virtual void draw(graphic &gr, rect clip) = 0;
    
    virtual void set_position(rect position) = 0;
    virtual rect position() const = 0;
    
    virtual void set_parent(std::shared_ptr<window> window_) = 0;
    virtual std::weak_ptr<window> parent() const = 0;
    virtual void clear_parent() = 0;
    
    virtual void set_topmost(bool yes) = 0;
    virtual bool topmost() const = 0;
    
    virtual void show() = 0;
    virtual void hide() = 0;
    virtual bool showed() const = 0;
    
    virtual void enable() = 0;
    virtual void disable() = 0;
    virtual bool enabled() const = 0;
    
    virtual bool focused() const = 0;
    virtual bool focusing() const = 0;
    
    virtual error get_error() const = 0;
};
```

## Adding Control to Window

```cpp
// Create control
auto button = std::make_shared<wui::button>("Click", []() {
    std::cout << "Clicked!" << std::endl;
});

// Add to window with position
window->add_control(button, {10, 10, 100, 30}); // left, top, right, bottom
```

## Theming

All controls support visual themes. Example theme JSON:

```json
{"controls": [
  {"type": "button", "calm": "#06a5df", "active": "#1aafe9", "disabled": "#a5a5a0"},
  {"type": "input", "background": "#ffffff", "text": "#000000"}
]}
```

[More about themes](../base/theme.md)

## See Also

- [Flat Ownership Model](../base/ownership.md)
- [Events](../base/event.md)
- [Visual Themes](../base/theme.md)
