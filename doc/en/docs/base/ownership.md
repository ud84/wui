# Control ownership

A window retains the controls passed to `add_control()`. Controls refer to their
parent through `weak_ptr`. Application classes commonly retain their own shared
references so they can update or reattach controls. `remove_control()` detaches a
control; it survives if another shared owner remains.

A panel is a visual surface, not a child-control container. An embedded `window`
can own its own controls. `message` and `tray_icon` are separate helper objects.

Avoid ownership cycles in callbacks. In particular, do not capture a control's
own shared pointer in its callback:

```cpp
auto check = std::make_shared<wui::button>("Remember", [] {}, wui::button_view::checkbox);
check->set_callback([weak = std::weak_ptr<wui::button>(check)] {
    if (auto control = weak.lock()) {
        bool checked = control->turned();
        // Apply checked to application state.
    }
});
```

Callbacks that capture `this` or references require the referenced objects to
outlive the subscription. Unsubscribe, stop timers and join workers before releasing
captured state. Retain dialogs until they are closed. See [threading](multi-threading.md).
