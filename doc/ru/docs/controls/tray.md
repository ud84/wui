# Иконка в трее (tray_icon)

Класс `tray_icon` добавляет иконку приложения в системный трей (область уведомлений).

## Быстрый старт

```cpp
#include <wui/control/tray_icon.hpp>

// Из файла
auto tray = std::make_shared<wui::tray_icon>(
    window,
    "res/icon.ico",
    "My Application",
    [](wui::tray_icon_action action) {
        switch (action) {
            case wui::tray_icon_action::left_click:
                show_window();
                break;
            case wui::tray_icon_action::right_click:
                show_context_menu();
                break;
            default:
                break;
        }
    }
);
```

## Типы действий

```cpp
enum class tray_icon_action
{
    left_click,    // Левый клик
    right_click,   // Правый клик
    center_click,  // Средний клик
    message_click  // Клик по сообщению
};
```

## API

### Конструкторы

```cpp
// Из ресурса (Windows)
#ifdef _WIN32
tray_icon(std::weak_ptr<window> parent, 
          int32_t icon_resource_index, 
          std::string_view tip, 
          std::function<void(tray_icon_action)> click_callback);
#endif

// Из файла
tray_icon(std::weak_ptr<window> parent, 
          std::string_view icon_file_name, 
          std::string_view tip, 
          std::function<void(tray_icon_action)> click_callback);
```

**Параметры:**
- `parent` — родительское окно
- `icon_resource_index` — ID ресурса иконки (Windows)
- `icon_file_name` — путь к файлу иконки
- `tip` — всплывающая подсказка
- `click_callback` — обработчик кликов

### Методы

```cpp
// Смена иконки
void change_icon(int32_t icon_resource_index);  // Windows
void change_icon(std::string_view icon_file_name);

// Смена подсказки
void change_tip(std::string_view tip);

// Показать уведомление
void show_message(std::string_view title, std::string_view message);

// Установить коллбэк
void set_callback(std::function<void(tray_icon_action)> click_callback);
```

## Примеры

### Базовая иконка в трее

```cpp
auto tray = std::make_shared<wui::tray_icon>(
    window,
#ifdef _WIN32
    IDI_APP_ICON,
#else
    "res/app_icon.png",
#endif
    "My Application - Running",
    [](wui::tray_icon_action action) {
        if (action == wui::tray_icon_action::left_click) {
            show_main_window();
        }
    }
);
```

### Контекстное меню по правому клику

```cpp
auto tray_menu = std::make_shared<wui::menu>();

wui::menu_items_t items = {
    {1, wui::normal, "Show", "", nullptr, {}, [](int32_t) {
        show_window();
    }},
    {2, wui::normal, "Hide", "", nullptr, {}, [](int32_t) {
        hide_window();
    }},
    {3, wui::separator, "", "", nullptr, {}, nullptr},
    {4, wui::normal, "Exit", "", nullptr, {}, [](int32_t) {
        exit_application();
    }}
};

tray_menu->set_items(items);

auto tray = std::make_shared<wui::tray_icon>(
    window,
    "res/icon.ico",
    "My Application",
    [&tray_menu](wui::tray_icon_action action) {
        if (action == wui::tray_icon_action::right_click) {
            tray_menu->show_on_point(get_cursor_position());
        } else if (action == wui::tray_icon_action::left_click) {
            show_window();
        }
    }
);
```

### Уведомления

```cpp
auto tray = std::make_shared<wui::tray_icon>(
    window,
    "res/icon.ico",
    "My Application",
    [](wui::tray_icon_action) {}
);

// Показать уведомление о завершении задачи
void on_task_complete() {
    tray->show_message(
        "Task Complete",
        "Your download has finished."
    );
}

// Показать уведомление об ошибке
void on_error() {
    tray->show_message(
        "Error",
        "Connection failed. Please check your network."
    );
}
```

### Смена иконки в зависимости от состояния

```cpp
auto tray = std::make_shared<wui::tray_icon>(
    window,
    "res/icon_normal.png",
    "My Application",
    [](wui::tray_icon_action action) {
        if (action == wui::tray_icon_action::left_click) {
            toggle_window();
        }
    }
);

// При изменении состояния
void on_status_changed(bool is_busy) {
    if (is_busy) {
        tray->change_icon("res/icon_busy.png");
        tray->change_tip("My Application - Processing");
    } else {
        tray->change_icon("res/icon_normal.png");
        tray->change_tip("My Application - Idle");
    }
}
```

### Иконка с ресурсом (Windows)

```cpp
// resource.h
#define IDI_APP_ICON  100
#define IDI_BUSY_ICON 101
#define IDI_ERROR_ICON 102

// В rc-файле
IDI_APP_ICON ICON "res/icon.ico"
IDI_BUSY_ICON ICON "res/busy.ico"
IDI_ERROR_ICON ICON "res/error.ico"

// В коде
#ifdef _WIN32
auto tray = std::make_shared<wui::tray_icon>(
    window,
    IDI_APP_ICON,
    "My Application",
    [](wui::tray_icon_action action) {
        // Обработка кликов
    }
);

// Смена иконки
void set_busy(bool busy) {
    tray->change_icon(busy ? IDI_BUSY_ICON : IDI_APP_ICON);
}
#endif
```

### Минимизация в трей

```cpp
auto tray = std::make_shared<wui::tray_icon>(
    window,
    "res/icon.ico",
    "My Application",
    [](wui::tray_icon_action action) {
        if (action == wui::tray_icon_action::left_click) {
            if (window->is_minimized()) {
                window->restore();
            } else {
                window->show();
            }
        }
    }
);

// При минимизации окна
window->set_minimize_callback([&tray]() {
    window->hide();
    tray->show_message("Minimized", "Application is running in tray");
});
```

## См. также

- [Меню](menu.md) — для контекстного меню
- [Окно](../base/interfaces.md#window) — родительское окно

## Поведение платформ

На macOS используется `NSStatusItem`; `show_message()` открывает popover, а не
уведомление Notification Center. В WASM системный трей не создаётся.
Ветви Linux этого класса пока являются заглушками; значок трея не создаётся.

Файловый конструктор Windows ожидает ICO; macOS принимает изображение, например PNG.
