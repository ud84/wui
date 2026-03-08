# Диалог сообщения (message)

Класс `message` предоставляет модальные диалоги для отображения сообщений пользователю.

## Быстрый старт

```cpp
#include <wui/control/message.hpp>

// Простое информационное сообщение
auto msg = std::make_shared<wui::message>(main_window);
msg->show(
    "Operation completed successfully",
    "Information",
    wui::message_icon::information,
    wui::message_button::ok,
    [](wui::message_result result) {
        if (result == wui::message_result::ok) {
            // Пользователь нажал OK
        }
    }
);
```

## Типы иконок

```cpp
enum class message_icon
{
    information,  // Информация (i)
    question,     // Вопрос (?)
    alert,        // Предупреждение (!)
    stop          // Ошибка (X)
};
```

## Типы кнопок

```cpp
enum class message_button
{
    ok,                // [OK]
    ok_cancel,         // [OK] [Cancel]
    abort_retry_ignore,// [Abort] [Retry] [Ignore]
    yes_no,            // [Yes] [No]
    yes_no_cancel,     // [Yes] [No] [Cancel]
    retry_cancel,      // [Retry] [Cancel]
    cancel_try_continue// [Cancel] [Try] [Continue]
};
```

## Результаты

```cpp
enum class message_result
{
    undef,
    ok, cancel, yes, no,
    abort, retry, ignore,
    try_, continue_
};
```

![Диалог сообщения WUI](../img/message0.png)

## API

### Конструктор

```cpp
message(std::shared_ptr<window> transient_window_,
        bool docked_ = true,
        std::shared_ptr<i_theme> theme_ = nullptr);
```

**Параметры:**
- `transient_window_` — родительское окно
- `docked_` — прикреплено ли к окну
- `theme_` — тема

### Методы

```cpp
void show(std::string_view message_,
          std::string_view title_,
          message_icon icon_,
          message_button button_,
          std::function<void(message_result)> result_callback = [](message_result) {});

message_result get_result() const;
```

## Примеры

### Информационное сообщение

```cpp
auto msg = std::make_shared<wui::message>(window);
msg->show(
    "File saved successfully",
    "Save",
    wui::message_icon::information,
    wui::message_button::ok
);
```

### Подтверждение действия

```cpp
auto msg = std::make_shared<wui::message>(window);
msg->show(
    "Are you sure you want to delete this file?",
    "Confirm Delete",
    wui::message_icon::question,
    wui::message_button::yes_no,
    [](wui::message_result result) {
        if (result == wui::message_result::yes) {
            delete_file();
        }
    }
);
```

### Сообщение об ошибке

```cpp
auto msg = std::make_shared<wui::message>(window);
msg->show(
    "Failed to connect to server.\nPlease check your network connection.",
    "Connection Error",
    wui::message_icon::stop,
    wui::message_button::ok_cancel,
    [](wui::message_result result) {
        if (result == wui::message_result::ok) {
            retry_connection();
        }
    }
);
```

### Предупреждение

```cpp
auto msg = std::make_shared<wui::message>(window);
msg->show(
    "The file has been modified by another program.\nReload?",
    "File Changed",
    wui::message_icon::alert,
    wui::message_button::yes_no_cancel,
    [](wui::message_result result) {
        switch (result) {
            case wui::message_result::yes:
                reload_file();
                break;
            case wui::message_result::no:
                ignore_changes();
                break;
            case wui::message_result::cancel:
                // Отмена
                break;
        }
    }
);
```

### Синхронный вызов

```cpp
auto msg = std::make_shared<wui::message>(window);
msg->show(
    "Continue?",
    "Confirm",
    wui::message_icon::question,
    wui::message_button::yes_no
);

if (msg->get_result() == wui::message_result::yes) {
    // Пользователь согласился
    proceed();
}
```

## Темизация

Сообщение использует темы для следующих контролов:

```json
{
  "type": "message",
  "background": "#ffffff",
  "text": "#000000",
  "button": "button",
  "icon_information": "res/info.png",
  "icon_question": "res/question.png",
  "icon_alert": "res/alert.png",
  "icon_stop": "res/stop.png"
}
```

## См. также

- [Кнопка](button.md)
- [Изображение](image.md)
- [Визуальные темы](../base/theme.md)
