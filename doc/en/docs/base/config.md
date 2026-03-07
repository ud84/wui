# Application Config

The Config subsystem provides a unified interface for storing application settings. On Windows, it uses the registry; on Linux/macOS, it uses INI files.

## Quick Start

```cpp
#include <wui/config/config.hpp>

// Initialize (Linux/macOS)
wui::config::use_ini_file("app.ini");

// Or on Windows
#ifdef _WIN32
wui::config::use_registry("Software\\MyApp\\WUI");
#endif

// Save setting
wui::config::set_int("window", "width", 800);
wui::config::set_string("user", "name", "Alex");

// Read setting
int width = wui::config::get_int("window", "width", 640); // 800
std::string name = wui::config::get_string("user", "name", "Unknown"); // "Alex"
```

## API

### use_ini_file
```cpp
bool use_ini_file(std::string_view file_name);
```

**Parameters:**
- `file_name` — path to INI file

**Returns:** `true` on success

Used on Linux/macOS or when file-based storage is required.

### use_registry (Windows only)
```cpp
bool use_registry(std::string_view app_key, HKEY root = HKEY_CURRENT_USER);
```

**Parameters:**
- `app_key` — registry key (e.g., "Software\\MyApp\\WUI")
- `root` — root registry key (default: `HKEY_CURRENT_USER`)

**Returns:** `true` on success

Used on Windows for storing settings in the registry.

### create_config
```cpp
bool create_config(std::string_view file_name, std::string_view app_key, 
                   int64_t root = 0);
```

**Parameters:**
- `file_name` — file name (for Linux/macOS)
- `app_key` — registry key (for Windows)
- `root` — root key (default: `HKEY_CURRENT_USER`)

**Returns:** `true` on success

Universal function for creating configuration.

### get_int / set_int
```cpp
int32_t get_int(std::string_view section, std::string_view entry, int32_t default_);
void set_int(std::string_view section, std::string_view entry, int32_t value);
```

**Parameters:**
- `section` — section name (e.g., "window", "user")
- `entry` — entry key
- `default_` — default value (for `get_int`)
- `value` — value to set (for `set_int`)

**Example:**
```cpp
wui::config::set_int("window", "width", 1024);
int width = wui::config::get_int("window", "width", 800);
```

### get_int64 / set_int64
```cpp
int64_t get_int64(std::string_view section, std::string_view entry, int64_t default_);
void set_int64(std::string_view section, std::string_view entry, int64_t value);
```

Similar to `get_int`/`set_int` for 64-bit values.

### get_string / set_string
```cpp
std::string get_string(std::string_view section, std::string_view entry, 
                       std::string_view default_);
void set_string(std::string_view section, std::string_view entry, 
                std::string_view value);
```

**Parameters:**
- `section` — section
- `entry` — entry key
- `default_` — default value (for `get_string`)
- `value` — string to set (for `set_string`)

**Example:**
```cpp
wui::config::set_string("user", "language", "ru");
std::string lang = wui::config::get_string("user", "language", "en");
```

### delete_value
```cpp
void delete_value(std::string_view section, std::string_view entry);
```

**Parameters:**
- `section` — section
- `entry` — entry key

Deletes a value from the configuration.

### delete_key
```cpp
void delete_key(std::string_view section);
```

**Parameters:**
- `section` — section

Deletes an entire section from the configuration.

### get_error
```cpp
error get_error();
```

**Returns:** The last config subsystem error

## INI File Format

Example configuration file:

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

## Usage Example

```cpp
#include <wui/config/config.hpp>
#include <wui/window/window.hpp>

class MainWindow : public wui::window
{
public:
    MainWindow()
    {
        // Initialize configuration
#ifdef _WIN32
        wui::config::use_registry("Software\\MyApp\\WUI");
#else
        wui::config::use_ini_file("wui_app.ini");
#endif

        // Load saved window size
        int width = wui::config::get_int("window", "width", 800);
        int height = wui::config::get_int("window", "height", 600);
        int x = wui::config::get_int("window", "x", -1);
        int y = wui::config::get_int("window", "y", -1);

        init("My Application", {x, y, width, height}, 
             wui::window_style::border_all, []() {
            // Save size on close
            // (in real code, get actual size)
            wui::config::set_int("window", "width", 800);
            wui::config::set_int("window", "height", 600);
        });
    }
};
```

## See Also

- [Locale](locale.md) — for multi-language support
- [Config](config.md) — for application settings storage
