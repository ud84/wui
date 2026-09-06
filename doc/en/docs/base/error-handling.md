# Error handling

Many WUI operations report failure through `bool` and an `error` value:

```cpp
if (!window->init("Application", {-1, -1, 800, 600}, wui::window_style::frame)) {
    std::cerr << window->get_error().str() << '\n';
}
```

Check a control's `get_error()` after loading images or constructing resources.
`error::is_ok()` checks the status; `str()` formats the component and message.
Theme/locale helpers also expose their load errors, and name-based loading accepts
an `error&` argument.

Do not interpret this as a guarantee that C++ exceptions can never occur: allocation,
user callbacks and some supporting APIs may throw. The WASM build explicitly enables
exception support. Handle application failures at the appropriate boundary.
