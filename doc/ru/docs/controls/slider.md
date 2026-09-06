# Ползунок (slider)

Контрол `slider` предоставляет ползунок для выбора значения из диапазона.

## Быстрый старт

```cpp
#include <wui/control/slider.hpp>

// Горизонтальный слайдер
auto slider = std::make_shared<wui::slider>(
    0,   // от
    100, // до
    50,  // текущее значение
    [](int32_t value) {
        std::cout << "Value: " << value << std::endl;
    }
);
window->add_control(slider, {10, 10, 200, 30});

// Вертикальный слайдер
auto v_slider = std::make_shared<wui::slider>(
    0, 100, 50,
    [](int32_t value) {
        set_volume(value);
    },
    wui::slider_orientation::vertical
);
window->add_control(v_slider, {10, 50, 30, 200});
```

## Ориентация

![Слайдер WUI](../img/slider.png)

```cpp
enum class slider_orientation
{
    vertical,   // Вертикальный
    horizontal  // Горизонтальный
};
```

## API

### Конструктор

```cpp
slider(int32_t from,
       int32_t to,
       int32_t value,
       std::function<void(int32_t)> change_callback,
       slider_orientation orientation = slider_orientation::horizontal,
       std::string_view theme_control_name = tc, 
       std::shared_ptr<i_theme> theme_ = nullptr);
```

**Параметры:**
- `from` — начало диапазона
- `to` — конец диапазона
- `value` — текущее значение
- `change_callback` — коллбэк при изменении
- `orientation` — ориентация
- `theme_control_name` — имя в теме
- `theme_` — тема

### Методы

```cpp
// Диапазон
void set_range(int32_t from, int32_t to);

// Значение
void set_value(int32_t value);
int32_t get_value() const;

// Режим с 0 посередине (для диапазонов типа -7..+7)
void set_centered_mode(bool centered);

// Коллбэки
void set_callback(std::function<void(int32_t)> change_callback);
void set_drag_end_callback(std::function<void()> drag_end_callback);
```

## Примеры

### Регулятор громкости

```cpp
auto volume_slider = std::make_shared<wui::slider>(
    0, 100, 50,
    [](int32_t value) {
        set_audio_volume(value);
    },
    wui::slider_orientation::vertical
);

volume_slider->set_drag_end_callback([]() {
    // Сохраняем значение после окончания перетаскивания
    save_volume_setting();
});

window->add_control(volume_slider, {10, 10, 40, 200});
```

### Выбор прозрачности

```cpp
auto opacity_slider = std::make_shared<wui::slider>(
    0, 100, 100,
    [&window](int32_t value) {
        window->set_opacity(value / 100.0);
    }
);

window->add_control(opacity_slider, {10, 10, 200, 30});
```

### Слайдер с отрицательными значениями

```cpp
// Диапазон от -10 до +10 с 0 посередине
auto balance_slider = std::make_shared<wui::slider>(
    -10, 10, 0,
    [](int32_t value) {
        set_audio_balance(value);
    }
);

balance_slider->set_centered_mode(true);

window->add_control(balance_slider, {10, 10, 200, 30});
```

### Слайдер с обновлением UI

```cpp
auto brightness_slider = std::make_shared<wui::slider>(
    0, 100, 50,
    [](int32_t value) {
        set_brightness(value);
    }
);

auto value_label = std::make_shared<wui::text>("50");

brightness_slider->set_callback([&value_label](int32_t value) {
    value_label->set_text(std::to_string(value));
});

window->add_control(brightness_slider, {10, 10, 200, 30});
window->add_control(value_label, {220, 10, 260, 30});
```

### Двойной слайдер (диапазон)

```cpp
// Минимальное значение
auto min_slider = std::make_shared<wui::slider>(
    0, 100, 20,
    [&max_slider](int32_t value) {
        if (value >= max_slider->get_value()) {
            max_slider->set_value(value + 1);
        }
    }
);

// Максимальное значение
auto max_slider = std::make_shared<wui::slider>(
    0, 100, 80,
    [&min_slider](int32_t value) {
        if (value <= min_slider->get_value()) {
            min_slider->set_value(value - 1);
        }
    }
);

window->add_control(min_slider, {10, 10, 200, 30});
window->add_control(max_slider, {10, 50, 200, 80});
```

## Темизация

```json
{
  "type": "slider",
  "perform": "#06a5df",
  "remain": "#e0e0e0",
  "active": "#1aafe9",
  "slider_width": 20,
  "slider_height": 12,
  "slider_round": 6
}
```

**Параметры:**
- `perform` — цвет заполненной части
- `remain` — цвет пустой части
- `active` — цвет при наведении
- `slider_width` — ширина ползунка
- `slider_height` — высота ползунка
- `slider_round` — скругление ползунка

## См. также

- [Progress](progress.md) — индикатор прогресса
- [Визуальные темы](../base/theme.md)
