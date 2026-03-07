# Flat Control Ownership Model

One of the key principles of working with WUI is the **flat control ownership model**. This model simplifies window and control lifecycle management, making code more understandable and predictable.

## Architectural Principle: Flat Ownership

In libwui, UI components are siblings within a logical controller (Dialog or Frame), rather than forming a ownership hierarchy.

### Key Features

**No Circular References**  
Controls don't own each other. All controls are owned by a single container object (usually a dialog class).

**RAII**  
Control lifecycles are tied to the owning object's lifecycle. As long as the logic object lives — the interface elements live too.

**Window Doesn't Own Controls**  
The `window->add_control(button, rect)` method doesn't transfer ownership to the window. The window stores non-owning references to controls for rendering and event forwarding.

**Control Relocation**  
A control can be moved from one window to another by simply calling `add_control` in the new window, without changing the ownership structure.

### Comparison with Other Approaches

Unlike Qt with hierarchical ownership via `QObject::parent`, where deleting a parent automatically deletes children, in libwui ownership is explicit and controlled by the developer.

### Example

```cpp
void DialingDialog::Cancel() {
    window->destroy(); // Window destroyed
    // cancelButton is still accessible in DialingDialog memory
    // you can read its state or perform additional logic
}
```

## Basic Principle

All window controls are owned by a **single parent object** (usually a dialog or window class), which stores them as `std::shared_ptr`. When the parent is destroyed, all controls are automatically released.

### Advantages of the Flat Model

- **Simple memory management** — no need to manually delete controls
- **Predictable lifecycle** — all controls are created and destroyed together with the window
- **No circular references** — controls don't own each other
- **Safety** — uses `std::shared_ptr`/`std::weak_ptr` for lifetime management

## Usage Pattern

Consider a typical pattern using a dialog example:

```cpp
class DialingDialog
{
public:
    DialingDialog(std::weak_ptr<wui::window> transientWindow, 
                  std::function<void()> cancelCallback);
    ~DialingDialog();
    
    void Run(std::string_view subscriber);
    void End();

private:
    static const int32_t WND_WIDTH = 300;
    static const int32_t WND_HEIGHT = 150;
    
    std::weak_ptr<wui::window> transientWindow;
    std::function<void()> cancelCallback;
    
    // Flat control ownership
    std::shared_ptr<wui::window> window;
    std::shared_ptr<wui::text> text;
    std::shared_ptr<wui::image> image;
    std::shared_ptr<wui::button> cancelButton;
    
    void Cancel();
};
```

## Creating Controls

Controls are created in the `Run()` method and immediately added to the window:

```cpp
void DialingDialog::Run(std::string_view subscriber)
{
    window->set_transient_for(transientWindow.lock());
    
    // Creating controls
    text = std::make_shared<wui::text>();
    image = std::make_shared<wui::image>(IMG_CALLOUT);
    cancelButton = std::make_shared<wui::button>(
        wui::locale("button", "cancel"), 
        std::bind(&DialingDialog::Cancel, this)
    );
    
    // Adding controls to the window
    window->add_control(text, { 84, 10, WND_WIDTH - 10, 65 });
    window->add_control(image, { 10, 10, 74, 74 });
    window->add_control(cancelButton, { 
        WND_WIDTH - 110, WND_HEIGHT - 40, 
        WND_WIDTH - 10, WND_HEIGHT - 10 
    });
    
    window->set_default_push_control(cancelButton);
    
    // Window initialization with close callback
    window->init("", { -1, -1, WND_WIDTH, WND_HEIGHT }, 
                 wui::window_style::border_all, 
                 [this]() {
                     // Releasing controls when window closes
                     cancelButton.reset();
                     image.reset();
                     text.reset();
                 });
    
    text->set_text(wui::locale("dialing_dialog", "dial") + 
                   " " + std::string(subscriber));
}
```

## Destroying Controls

Controls are released in the window close callback passed to `init()`:

```cpp
window->init("", { -1, -1, WND_WIDTH, WND_HEIGHT }, 
             wui::window_style::border_all, 
             [this]() {
                 cancelButton.reset();
                 image.reset();
                 text.reset();
             });
```

This ensures all controls are properly destroyed when the window closes.

## Dialog Completion Methods

```cpp
void DialingDialog::End()
{
    if (window) {
        window->destroy();
    }
}

void DialingDialog::Cancel()
{
    if (window) {
        window->destroy();
    }
    cancelCallback();
}
```

## Important Notes

1. **Don't use nested ownership** — controls should not own each other
2. **Use `std::weak_ptr`** for temporary window references (e.g., `transientWindow`)
3. **Always release controls** in the window close callback
4. **Check validity** before use (`if (window)`)

## Common Mistakes

### ❌ Incorrect: Control Owns Another Control

```cpp
// panel holds button internally as shared_ptr
class BadDialog {
    std::shared_ptr<wui::panel> panel;
    // panel->add_control(button) creates ownership inside panel
    // When destroying dialog -> panel -> button, problems occur
};
```

Problem: If `panel` owns `button` internally, and `dialog` also owns `button` — undefined behavior occurs during destruction.

### ✅ Correct: All Controls Owned by Single Object

```cpp
class GoodDialog {
    std::shared_ptr<wui::window> window;
    std::shared_ptr<wui::button> button;
    std::shared_ptr<wui::panel> panel;
    
    GoodDialog() {
        window = std::make_shared<wui::window>();
        button = std::make_shared<wui::button>("Click", [](){});
        panel = std::make_shared<wui::panel>();
        
        // add_control doesn't take ownership, only a reference
        window->add_control(button, {10, 10, 100, 30});
        window->add_control(panel, {0, 0, 200, 200});
    }
};
```

All controls are owned by `GoodDialog`, the window only references them.

## Conclusion

The flat control ownership model is a simple and reliable way to manage UI element lifecycles in WUI. Following this pattern helps avoid memory leaks, circular references, and undefined behavior.
