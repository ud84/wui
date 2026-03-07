# Конфигурация приложения (Config)

Подсистема Config предоставляет единый интерфейс для хранения настроек приложения. На Windows используется реестр, на Linux/macOS — INI-файлы.

## Быстрый старт

```cpp
#include <wui/config/config.hpp>

// Инициализация (Linux/macOS)
wui::config::use_ini_file("app.ini");

// Или на Windows
#ifdef _WIN32
wui::config::use_registry("Software\\MyApp\\WUI");
#endif

// Сохранение настройки
wui::config::set_int("window", "width", 800);
wui::config::set_string("user", "name", "Alex");

// Чтение настройки
int width = wui::config::get_int("window", "width", 640); // 800
std::string name = wui::config::get_string("user", "name", "Unknown"); // "Alex"
```

## API

### use_ini_file
```cpp
bool use_ini_file(std::string_view file_name);
```

**Параметры:**
- `file_name` — путь к INI-файлу

**Возвращает:** `true` в случае успеха

Используется на Linux/macOS или когда требуется файловое хранение.

### use_registry (Windows only)
```cpp
bool use_registry(std::string_view app_key, HKEY root = HKEY_CURRENT_USER);
```

**Параметры:**
- `app_key` — ключ реестра (например, "Software\\MyApp\\WUI")
- `root` — корневой ключ реестра (по умолчанию `HKEY_CURRENT_USER`)

**Возвращает:** `true` в случае успеха

Используется на Windows для хранения настроек в реестре.

### create_config
```cpp
bool create_config(std::string_view file_name, std::string_view app_key, 
                   int64_t root = 0);
```

**Параметры:**
- `file_name` — имя файла (для Linux/macOS)
- `app_key` — ключ реестра (для Windows)
- `root` — корневой ключ (по умолчанию `HKEY_CURRENT_USER`)

**Возвращает:** `true` в случае успеха

Универсальная функция для создания конфигурации.

### get_int / set_int
```cpp
int32_t get_int(std::string_view section, std::string_view entry, int32_t default_);
void set_int(std::string_view section, std::string_view entry, int32_t value);
```

**Параметры:**
- `section` — секция (например, "window", "user")
- `entry` — ключ записи
- `default_` — значение по умолчанию (для `get_int`)
- `value` — устанавливаемое значение (для `set_int`)

**Пример:**
```cpp
wui::config::set_int("window", "width", 1024);
int width = wui::config::get_int("window", "width", 800);
```

### get_int64 / set_int64
```cpp
int64_t get_int64(std::string_view section, std::string_view entry, int64_t default_);
void set_int64(std::string_view section, std::string_view entry, int64_t value);
```

Аналогично `get_int`/`set_int` для 64-битных значений.

### get_string / set_string
```cpp
std::string get_string(std::string_view section, std::string_view entry, 
                       std::string_view default_);
void set_string(std::string_view section, std::string_view entry, 
                std::string_view value);
```

**Параметры:**
- `section` — секция
- `entry` — ключ записи
- `default_` — значение по умолчанию (для `get_string`)
- `value` — устанавливаемая строка (для `set_string`)

**Пример:**
```cpp
wui::config::set_string("user", "language", "ru");
std::string lang = wui::config::get_string("user", "language", "en");
```

### delete_value
```cpp
void delete_value(std::string_view section, std::string_view entry);
```

**Параметры:**
- `section` — секция
- `entry` — ключ записи

Удаляет значение из конфигурации.

### delete_key
```cpp
void delete_key(std::string_view section);
```

**Параметры:**
- `section` — секция

Удаляет всю секцию из конфигурации.

### get_error
```cpp
error get_error();
```

**Возвращает:** Последнюю ошибку подсистемы конфигурации

## Формат INI-файла

Пример файла конфигурации:

```ini
[window]
width=1024
height=768
x=100
y=50

[user]
name=Alex
language=ru
theme=dark

[settings]
auto_save=true
backup_count=5
```

## Пример использования

```cpp
#include <wui/config/config.hpp>
#include <wui/window/window.hpp>

class MainWindow : public wui::window
{
public:
    MainWindow()
    {
        // Инициализация конфигурации
#ifdef _WIN32
        wui::config::use_registry("Software\\MyApp\\WUI");
#else
        wui::config::use_ini_file("wui_app.ini");
#endif

        // Загрузка сохранённых размеров окна
        int width = wui::config::get_int("window", "width", 800);
        int height = wui::config::get_int("window", "height", 600);
        int x = wui::config::get_int("window", "x", -1);
        int y = wui::config::get_int("window", "y", -1);

        init("My Application", {x, y, width, height}, 
             wui::window_style::border_all, []() {
            // Сохранение размеров при закрытии
            // (в реальном коде получите актуальные размеры)
            wui::config::set_int("window", "width", 800);
            wui::config::set_int("window", "height", 600);
        });
    }
};
```

## См. также

- [Локали](locale.md) — для многоязыковой поддержки
- [Конфигурация](config.md) — для хранения настроек
