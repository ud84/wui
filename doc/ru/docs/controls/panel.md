# Панель

Панель рисует фон из темы и необязательную пользовательскую графику. Это не
контейнер и не автоматическая компоновка. Связанные контролы добавляйте в то же
окно после панели. Координаты рисования относятся к окну.

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

При динамической компоновке берите актуальные границы из состояния приложения
в callback. В `graphic` нет `draw_circle()`: выше используется полностью скруглённый
квадрат. Параметр темы панели — `background`. См. [графику](../base/graphic.md) и [splitter](splitter.md).
