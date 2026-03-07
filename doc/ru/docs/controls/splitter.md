# Разделитель (splitter)

Контрол `splitter` предоставляет перетаскиваемый разделитель для изменения размера областей.

## Быстрый старт

```cpp
#include <wui/control/splitter.hpp>

// Вертикальный разделитель
auto splitter = std::make_shared<wui::splitter>(
    wui::splitter_orientation::vertical,
    [](int32_t x, int32_t y) {
        std::cout << "Splitter moved to: " << x << ", " << y << std::endl;
    }
);
window->add_control(splitter, {200, 10, 210, 400});
```

## Ориентация

![Разделитель WUI](../img/splitter.png)

```cpp
enum class splitter_orientation
{
    vertical,   // Вертикальный (разделяет слева/справа)
    horizontal  // Горизонтальный (разделяет сверху/снизу)
};
```

## API

### Конструктор

```cpp
splitter(splitter_orientation orientation, 
         std::function<void(int32_t, int32_t)> callback, 
         std::string_view theme_control_name = tc, 
         std::shared_ptr<i_theme> theme_ = nullptr);
```

**Параметры:**
- `orientation` — ориентация разделителя
- `callback` — вызывается при перемещении с координатами
- `theme_control_name` — имя в теме
- `theme_` — тема

### Методы

```cpp
// Коллбэк
void set_callback(std::function<void(int32_t, int32_t)> callback_);

// Ограничения перемещения
void set_margins(int32_t min_, int32_t max_);
```

## Примеры

### Разделение окна на две панели

```cpp
// Левая панель
auto left_panel = std::make_shared<wui::panel>();
window->add_control(left_panel, {0, 0, 200, 500});

// Разделитель
auto splitter = std::make_shared<wui::splitter>(
    wui::splitter_orientation::vertical,
    [&left_panel](int32_t x, int32_t y) {
        // Изменяем размер левой панели
        auto pos = left_panel->position();
        left_panel->set_position({pos.left, pos.top, x, pos.bottom});
    }
);
window->add_control(splitter, {200, 0, 210, 500});

// Правая панель
auto right_panel = std::make_shared<wui::panel>();
window->add_control(right_panel, {210, 0, 500, 500});
```

### Горизонтальное разделение

```cpp
// Верхняя панель
auto top_panel = std::make_shared<wui::panel>();
window->add_control(top_panel, {0, 0, 500, 200});

// Горизонтальный разделитель
auto h_splitter = std::make_shared<wui::splitter>(
    wui::splitter_orientation::horizontal,
    [&top_panel](int32_t x, int32_t y) {
        auto pos = top_panel->position();
        top_panel->set_position({pos.left, pos.top, pos.right, y});
    }
);
window->add_control(h_splitter, {0, 200, 500, 210});

// Нижняя панель
auto bottom_panel = std::make_shared<wui::panel>();
window->add_control(bottom_panel, {0, 210, 500, 500});
```

### Разделитель с ограничениями

```cpp
auto constrained_splitter = std::make_shared<wui::splitter>(
    wui::splitter_orientation::vertical,
    [](int32_t x, int32_t y) {
        resize_panels(x, y);
    }
);

// Ограничиваем перемещение от 100 до 400 пикселей
constrained_splitter->set_margins(100, 400);

window->add_control(constrained_splitter, {200, 0, 210, 500});
```

### Сложная компоновка с несколькими разделителями

```cpp
// Создаём сетку 2x2 с разделителями

// Верх-лево
auto top_left = std::make_shared<wui::panel>();
window->add_control(top_left, {0, 0, 250, 200});

// Вертикальный разделитель
auto v_splitter = std::make_shared<wui::splitter>(
    wui::splitter_orientation::vertical,
    [](int32_t x, int32_t y) { /* ... */ }
);
window->add_control(v_splitter, {250, 0, 260, 200});

// Верх-право
auto top_right = std::make_shared<wui::panel>();
window->add_control(top_right, {260, 0, 500, 200});

// Горизонтальный разделитель
auto h_splitter = std::make_shared<wui::splitter>(
    wui::splitter_orientation::horizontal,
    [](int32_t x, int32_t y) { /* ... */ }
);
window->add_control(h_splitter, {0, 200, 500, 210});

// Низ-лево
auto bottom_left = std::make_shared<wui::panel>();
window->add_control(bottom_left, {0, 210, 250, 500});

// Нижний вертикальный разделитель
auto v_splitter2 = std::make_shared<wui::splitter>(
    wui::splitter_orientation::vertical,
    [](int32_t x, int32_t y) { /* ... */ }
);
window->add_control(v_splitter2, {250, 210, 260, 500});

// Низ-право
auto bottom_right = std::make_shared<wui::panel>();
window->add_control(bottom_right, {260, 210, 500, 500});
```

## Темизация

```json
{
  "type": "splitter",
  "calm": "#cccccc",
  "active": "#06a5df"
}
```

**Параметры:**
- `calm` — цвет в покое
- `active` — цвет при наведении/перетаскивании

## См. также

- [Панель](panel.md) — для разделяемых областей
- [Визуальные темы](../base/theme.md)
