# Подсказка (tooltip)

Контрол `tooltip` предоставляет всплывающие подсказки для элементов интерфейса.

## Быстрый старт

```cpp
#include <wui/control/tooltip.hpp>

// Создание подсказки
auto tooltip = std::make_shared<wui::tooltip>("This is a helpful tooltip");

// Показать над кнопкой
window->add_control(tooltip, {0});
tooltip->hide();

auto button = std::make_shared<wui::button>("Show tooltip", []() {});
window->add_control(button, {10, 10, 100, 30});

// Показываем подсказку при наведении
button->set_callback([&tooltip, &button]() {
    tooltip->show_on_control(*button, 5);
});
```

## API

![Подсказка WUI](../img/tooltip.png)

### Конструктор

```cpp
tooltip(std::string_view text, 
        std::string_view theme_control_name = tc, 
        std::shared_ptr<i_theme> theme_ = nullptr);
```

**Параметры:**
- `text` — текст подсказки
- `theme_control_name` — имя в теме
- `theme_` — тема

### Методы

```cpp
// Показать над контролом
void show_on_control(i_control &control, int32_t indent);

// Установить текст
void set_text(std::string_view text_);
```

## Примеры

### Подсказка для кнопки

```cpp
auto save_button = std::make_shared<wui::button>("Save", []() {
    save_file();
});

auto save_tooltip = std::make_shared<wui::tooltip>(
    "Save the current document (Ctrl+S)"
);

// Показываем при наведении
save_button->set_callback([&save_button, &save_tooltip]() {
    save_tooltip->show_on_control(*save_button, 5);
    save_tooltip->show();
});

window->add_control(save_button, {10, 10, 100, 30});
```

### Подсказка для поля ввода

```cpp
auto email_input = std::make_shared<wui::input>("Enter email");

auto email_tooltip = std::make_shared<wui::tooltip>(
    "Please enter a valid email address\nExample: user@example.com"
);

email_input->set_change_callback([&email_input, &email_tooltip]() {
    if (email_input->text().empty()) {
        email_tooltip->set_text("Email is required");
        email_tooltip->show_on_control(*email_input, 5);
        email_tooltip->show();
    }
});

window->add_control(email_input, {10, 10, 200, 30});
```

### Многострочная подсказка

```cpp
auto complex_tooltip = std::make_shared<wui::tooltip>(
    "Keyboard Shortcuts:\n"
    "  Ctrl+S - Save\n"
    "  Ctrl+O - Open\n"
    "  Ctrl+N - New\n"
    "  F1 - Help"
);

auto help_button = std::make_shared<wui::button>("?", []() {
    complex_tooltip->show_on_control(*help_button, 5);
    complex_tooltip->show();
});

window->add_control(help_button, {10, 10, 30, 30});
```

### Динамическое обновление текста

```cpp
auto status_tooltip = std::make_shared<wui::tooltip>("Ready");

auto refresh_button = std::make_shared<wui::button>("Refresh", [&]() {
    status_tooltip->set_text("Loading...");
    status_tooltip->show_on_control(*refresh_button, 5);
    status_tooltip->show();
    
    load_data().then([&status_tooltip](auto result) {
        status_tooltip->set_text("Last updated: " + result.timestamp);
    });
});

window->add_control(refresh_button, {10, 10, 100, 30});
```

### Подсказка для иконки

```cpp
auto info_icon = std::make_shared<wui::image>("res/info.png");

auto info_tooltip = std::make_shared<wui::tooltip>(
    "Click for more information"
);

// Показываем при наведении мыши
window->subscribe([&info_icon, &info_tooltip](const wui::event& ev) {
    if (ev.type == wui::event_type::mouse && 
        ev.mouse_event_.type == wui::mouse_event_type::move) {
        
        auto pos = info_icon->position();
        if (ev.mouse_event_.x >= pos.left && ev.mouse_event_.x <= pos.right &&
            ev.mouse_event_.y >= pos.top && ev.mouse_event_.y <= pos.bottom) {
            info_tooltip->show_on_control(*info_icon, 5);
            info_tooltip->show();
        } else {
            info_tooltip->hide();
        }
    }
}, wui::event_type::mouse);

window->add_control(info_icon, {10, 10, 24, 24});
```

## Темизация

```json
{
  "type": "tooltip",
  "background": "#ffffe0",
  "border": "#d4d4aa",
  "border_width": 1,
  "text": "#000000",
  "text_indent": 5,
  "round": 4,
  "font": {"name": "Segoe UI", "size": 18}
}
```

**Параметры:**
- `background` — цвет фона
- `border` — цвет рамки
- `border_width` — толщина рамки
- `text` — цвет текста
- `text_indent` — отступ текста
- `round` — скругление углов
- `font` — шрифт

## См. также

- [Изображение](image.md)
- [Кнопка](button.md)
- [Визуальные темы](../base/theme.md)
