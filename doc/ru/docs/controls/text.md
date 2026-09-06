# Текст

Неизменяемая пользователем подпись. Горизонтальное и вертикальное выравнивание
задаются разными enum; старого `text_alignment` в текущем API нет.

```cpp
#include <wui/control/text.hpp>

auto label = std::make_shared<wui::text>("Hello, WUI!",
    wui::hori_alignment::center, wui::vert_alignment::center);
window->add_control(label, {20, 50, 300, 90});
label->set_text("Updated caption");
```

```cpp
text(std::string_view text = "",
     hori_alignment horizontal = hori_alignment::left,
     vert_alignment vertical = vert_alignment::center,
     std::string_view theme_control_name = tc,
     std::shared_ptr<i_theme> theme = nullptr);
void set_text(std::string_view text);
```

Раздел темы — `text`, параметры — `color` и объект `font`. Для редактирования
или выделения/копирования используйте [input](input.md).
