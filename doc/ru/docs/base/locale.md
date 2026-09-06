# Локали (Locale)

Подсистема Locale предоставляет удобный способ централизованного хранения текстового контента приложения для поддержки множественных языков.

## Быстрый старт

```cpp
#include <wui/locale/locale.hpp>

// Установка локали из файла (Linux/macOS)
auto ok = wui::set_locale_from_file(wui::locale_type::rus, "ru", "res/ru_locale.json");
if (!ok) {
    std::cerr << "can't load locale" << std::endl;
    return -1;
}

// Использование
std::string greeting = wui::locale("greeting", "hello"); // "Здравствуйте"
```

## API

### set_locale_from_file
```cpp
bool set_locale_from_file(locale_type type, std::string_view name, std::string_view file_name);
```

**Параметры:**
- `type` — тип локали (например, `locale_type::rus`, `locale_type::eng`)
- `name` — имя локали
- `file_name` — путь к JSON-файлу с локалью

**Возвращает:** `true` в случае успеха

### set_locale_from_json
```cpp
bool set_locale_from_json(locale_type type, std::string_view name, std::string_view json);
```

**Параметры:**
- `type` — тип локали
- `name` — имя локали
- `json` — JSON-строка с локалью

**Возвращает:** `true` в случае успеха

### set_locale_from_resource (Windows only)
```cpp
bool set_locale_from_resource(locale_type type, std::string_view name, 
                              int32_t resource_index, std::string_view resource_section);
```

**Параметры:**
- `type` — тип локали
- `name` — имя локали
- `resource_index` — ID ресурса
- `resource_section` — секция ресурсов (например, "JSONS")

**Возвращает:** `true` в случае успеха

### get_locale
```cpp
std::shared_ptr<i_locale> get_locale();
```

**Возвращает:** Указатель на текущий экземпляр локали по умолчанию

### locale
```cpp
const std::string &locale(std::string_view section, std::string_view value);
```

**Параметры:**
- `section` — секция (например, "button", "dialog")
- `value` — ключ значения (например, "cancel", "ok")

**Возвращает:** Ссылку на строку из текущей локали

**Пример:**
```cpp
std::string cancel_text = wui::locale("button", "cancel"); // "Отмена"
std::string title = wui::locale("dialog", "settings_title"); // "Настройки"
```

### set_locale_value
```cpp
void set_locale_value(std::string_view section, std::string_view value, std::string_view str);
```

**Параметры:**
- `section` — секция
- `value` — ключ
- `str` — устанавливаемая строка

Позволяет динамически устанавливать значения локали во время выполнения.

## Формат JSON

Пример файла локали:

```json
{
  "button": {
    "ok": "OK",
    "cancel": "Отмена",
    "apply": "Применить"
  },
  "dialog": {
    "settings_title": "Настройки",
    "about_title": "О программе"
  },
  "message": {
    "error": "Ошибка",
    "warning": "Предупреждение",
    "info": "Информация"
  }
}
```

## Интерфейс i_locale

```cpp
class i_locale
{
public:
    virtual std::string get_name() const = 0;

    virtual void set(std::string_view section, std::string_view value,
                          std::string_view str) = 0;
    virtual const std::string &get(std::string_view section,
                                        std::string_view value) const = 0;

#ifdef _WIN32
    virtual void load_resource(int32_t resource_index, 
                              std::string_view resource_section) = 0;
#endif
    virtual void load_json(std::string_view json) = 0;
    virtual void load_file(std::string_view file_name) = 0;

    virtual error get_error() const = 0;

    virtual ~i_locale() {}
};
```

### get_name
**Возвращает:** Имя экземпляра локали (например, "ru", "en")

### set
**Параметры:**
- `section` — секция
- `value` — ключ
- `str` — устанавливаемая строка

Устанавливает строковое значение для секции и ключа.

### get
**Параметры:**
- `section` — секция
- `value` — ключ

**Возвращает:** Ссылку на строковое значение

### load_json / load_file / load_resource
Загружает локаль из JSON-строки, файла или ресурса.

### get_error
Возвращает `error`, а не bool; проверяйте `is_ok()` и `str()`.

## Переключение языка

```cpp
wui::error error;
if (wui::set_locale_from_type(wui::get_next_app_locale(), error)) {
    label->set_text(wui::locale("main_frame", "whats_your_name_text"));
    button->set_caption(wui::locale("main_frame", "ok_button"));
}
```

Сначала зарегистрируйте локали через `set_app_locales()`, как в полном примере
Hello world. Загрузка локали не заменяет автоматически строки, уже сохранённые
в контролах. Состояние экземпляра локали проверяется через `get_error().is_ok()`.
