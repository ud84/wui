# Locale

The Locale subsystem provides a convenient way to centrally store text content for multi-language support.

## Quick Start

```cpp
#include <wui/locale/locale.hpp>

// Set locale from file (Linux/macOS)
auto ok = wui::set_locale_from_file(wui::locale_type::ru, "ru", "res/ru.json");
if (!ok) {
    std::cerr << "can't load locale" << std::endl;
    return -1;
}

// Usage
std::string greeting = wui::locale("greeting", "hello"); // "Hello"
```

## API

### set_locale_from_file
```cpp
bool set_locale_from_file(locale_type type, std::string_view name, std::string_view file_name);
```

**Parameters:**
- `type` — locale type (e.g., `locale_type::ru`, `locale_type::en`)
- `name` — locale name
- `file_name` — path to JSON locale file

**Returns:** `true` on success

### set_locale_from_json
```cpp
bool set_locale_from_json(locale_type type, std::string_view name, std::string_view json);
```

**Parameters:**
- `type` — locale type
- `name` — locale name
- `json` — JSON string with locale data

**Returns:** `true` on success

### set_locale_from_resource (Windows only)
```cpp
bool set_locale_from_resource(locale_type type, std::string_view name, 
                              int32_t resource_index, std::string_view resource_section);
```

**Parameters:**
- `type` — locale type
- `name` — locale name
- `resource_index` — resource ID
- `resource_section` — resource section (e.g., "JSONS")

**Returns:** `true` on success

### get_locale
```cpp
std::shared_ptr<i_locale> get_locale();
```

**Returns:** Pointer to the current default locale instance

### locale
```cpp
const std::string &locale(std::string_view section, std::string_view value);
```

**Parameters:**
- `section` — section name (e.g., "button", "dialog")
- `value` — value key (e.g., "cancel", "ok")

**Returns:** Reference to the string from current locale

**Example:**
```cpp
std::string cancel_text = wui::locale("button", "cancel"); // "Cancel"
std::string title = wui::locale("dialog", "settings_title"); // "Settings"
```

### set_locale_value
```cpp
void set_locale_value(std::string_view section, std::string_view value, std::string_view str);
```

**Parameters:**
- `section` — section
- `value` — key
- `str` — string to set

Allows dynamically setting locale values at runtime.

## JSON Format

Example locale file:

```json
{
  "button": {
    "ok": "OK",
    "cancel": "Cancel",
    "apply": "Apply"
  },
  "dialog": {
    "settings_title": "Settings",
    "about_title": "About"
  },
  "message": {
    "error": "Error",
    "warning": "Warning",
    "info": "Info"
  }
}
```

## i_locale Interface

```cpp
class i_locale
{
public:
    virtual std::string get_name() const = 0;

    virtual void set_value(std::string_view section, std::string_view value, 
                          std::string_view str) = 0;
    virtual const std::string &get_value(std::string_view section, 
                                        std::string_view value) const = 0;

#ifdef _WIN32
    virtual void load_resource(int32_t resource_index, 
                              std::string_view resource_section) = 0;
#endif
    virtual void load_json(std::string_view json) = 0;
    virtual void load_file(std::string_view file_name) = 0;

    virtual bool is_ok() const = 0;

    virtual ~i_locale() {}
};
```

### get_name
**Returns:** Locale instance name (e.g., "ru", "en")

### set_value
**Parameters:**
- `section` — section
- `value` — key
- `str` — string to set

Sets a string value for the section and key.

### get_value
**Parameters:**
- `section` — section
- `value` — key

**Returns:** Reference to the string value

### load_json / load_file / load_resource
Loads locale from JSON string, file, or resource.

### is_ok
**Returns:** `true` if locale loaded successfully

## Language Switching

To switch language at runtime:

```cpp
// Load English locale
wui::set_locale_from_file(wui::locale_type::en, "en", "res/en.json");

// All controls will automatically update text
// or update them manually if needed
```

## See Also

- [Visual Themes](theme.md) — for interface styling
- [Config](config.md) — for application settings storage
