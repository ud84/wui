# Image

The `image` control displays images with support for visual themes.

## Quick Start

```cpp
#include <wui/control/image.hpp>

// From file
auto logo = std::make_shared<wui::image>("res/logo.png");
window->add_control(logo, {10, 10, 100, 100});

// From resource (Windows)
#ifdef _WIN32
auto icon = std::make_shared<wui::image>(IDI_LOGO);
window->add_control(icon, {10, 10, 32, 32});
#endif

// From data
std::vector<uint8_t> png_data = load_png_file("icon.png");
auto img = std::make_shared<wui::image>(png_data);
window->add_control(img, {50, 50, 64, 64});
```

## API

### Constructors

```cpp
// From resource (Windows)
image(int32_t resource_index, std::shared_ptr<i_theme> theme_ = nullptr);

// From file
image(std::string_view file_name, std::shared_ptr<i_theme> theme_ = nullptr);

// From data
image(const std::vector<uint8_t> &data);
```

### Methods

```cpp
// Change image
void change_image(int32_t resource_index);  // Windows
void change_image(std::string_view file_name);
void change_image(const std::vector<uint8_t> &data);

// Size
int32_t width() const;
int32_t height() const;
```

## Working with Themes

`image` automatically supports theme switching. Settings are stored in the theme:

### light.json
```json
{
  "type": "image",
  "resource": "IMAGES_LIGHT",
  "path": "res/images/light"
}
```

### dark.json
```json
{
  "type": "image",
  "resource": "IMAGES_DARK",
  "path": "res/images/dark"
}
```

When the theme changes, images are automatically loaded from the corresponding directory or resource.

## Examples

### Application Logo

```cpp
// In window constructor
logo_ = std::make_shared<wui::image>("res/logo.png");
window->add_control(logo_, {10, 10, 128, 64});
```

### Icon from Resource (Windows)

```cpp
// resource.h
#define IMG_LOGO  109
#define IMG_SAVE  110

// In rc file
IMG_LOGO IMAGES_DARK   "res\\images\\dark\\logo.png"
IMG_LOGO IMAGES_LIGHT  "res\\images\\light\\logo.png"

// In code
#ifdef _WIN32
auto logo = std::make_shared<wui::image>(IMG_LOGO);
#endif
```

### Dynamic Image Change

```cpp
auto status_icon = std::make_shared<wui::image>("res/online.png");
window->add_control(status_icon, {10, 10, 16, 16});

// When status changes
if (is_online) {
    status_icon->change_image("res/online.png");
} else {
    status_icon->change_image("res/offline.png");
}
```

## Integration with Button

Buttons use `image` internally to display icons:

```cpp
auto save_btn = std::make_shared<wui::button>(
    "Save",
    []() { save(); },
    wui::button_view::image_right_text,
    "res/save.png",
    16
);
```

## See Also

- [Button](button.md) — uses image for icons
- [Visual Themes](../base/theme.md)
