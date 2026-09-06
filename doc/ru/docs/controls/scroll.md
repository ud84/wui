# Полоса прокрутки

Отдельная полоса сообщает смещение; отображением содержимого управляет приложение.
Списки и многострочные поля уже управляют своими полосами прокрутки.

```cpp
#include <wui/control/scroll.hpp>

auto bar = std::make_shared<wui::scroll>(1000, 0,
    wui::orientation::vertical,
    [](wui::scroll_state state, int32_t offset) {
        // Update the application's visible content for this offset.
    });
window->add_control(bar, {300, 50, 314, 350});
```

```cpp
scroll(int32_t area, int32_t scroll_pos,
       orientation orientation = orientation::vertical,
       std::function<void(scroll_state, int32_t)> callback = nullptr,
       std::string_view theme_control_name = tc,
       std::shared_ptr<i_theme> theme = nullptr);
```

```cpp
void set_area(int32_t area);
    void set_scroll_pos(int32_t scroll_pos);
    int32_t get_scroll_pos() const;

    /// Good to call from mouse whell event
    void scroll_up();
    void scroll_down();
```

Состояния: `activated`, `relaxed`, `up_end`, `down_end`, `moving`.
`get_scroll_view()` возвращает `none`, `tiny` или `full`; видимость меняется при
взаимодействии. Цвета раздела `scroll`: `background`, `slider`, `slider_active`.
