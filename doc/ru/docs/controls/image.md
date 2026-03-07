# Изображение (image)

Контрол `image` предназначен для отображения изображений с поддержкой визуальных тем.

## Быстрый старт

```cpp
#include <wui/control/image.hpp>

// Из файла
auto logo = std::make_shared<wui::image>("res/logo.png");
window->add_control(logo, {10, 10, 100, 100});

// Из ресурса (Windows)
#ifdef _WIN32
auto icon = std::make_shared<wui::image>(IDI_LOGO);
window->add_control(icon, {10, 10, 32, 32});
#endif

// Из данных
std::vector<uint8_t> png_data = load_png_file("icon.png");
auto img = std::make_shared<wui::image>(png_data);
window->add_control(img, {50, 50, 64, 64});
```

## API

### Конструкторы

```cpp
// Из ресурса (Windows)
image(int32_t resource_index, std::shared_ptr<i_theme> theme_ = nullptr);

// Из файла
image(std::string_view file_name, std::shared_ptr<i_theme> theme_ = nullptr);

// Из данных
image(const std::vector<uint8_t> &data);
```

### Методы

```cpp
// Смена изображения
void change_image(int32_t resource_index);  // Windows
void change_image(std::string_view file_name);
void change_image(const std::vector<uint8_t> &data);

// Размеры
int32_t width() const;
int32_t height() const;
```

## Работа с темами

`image` автоматически поддерживает смену тем. Настройки хранятся в теме:

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

При смене темы изображения автоматически подгружаются из соответствующей директории или ресурса.

## Примеры

### Логотип приложения

```cpp
// В конструкторе окна
logo_ = std::make_shared<wui::image>("res/logo.png");
window->add_control(logo_, {10, 10, 128, 64});
```

### Иконка с ресурсом (Windows)

```cpp
// resource.h
#define IMG_LOGO  109
#define IMG_SAVE  110

// В rc-файле
IMG_LOGO IMAGES_DARK   "res\\images\\dark\\logo.png"
IMG_LOGO IMAGES_LIGHT  "res\\images\\light\\logo.png"

// В коде
#ifdef _WIN32
auto logo = std::make_shared<wui::image>(IMG_LOGO);
#endif
```

### Динамическая смена изображения

```cpp
auto status_icon = std::make_shared<wui::image>("res/online.png");
window->add_control(status_icon, {10, 10, 16, 16});

// При изменении статуса
if (is_online) {
    status_icon->change_image("res/online.png");
} else {
    status_icon->change_image("res/offline.png");
}
```

## Интеграция с button

Кнопки используют `image` internally для отображения иконок:

```cpp
auto save_btn = std::make_shared<wui::button>(
    "Save",
    []() { save(); },
    wui::button_view::image_right_text,
    "res/save.png",
    16
);
```

## См. также

- [Кнопка](button.md) — использует image для иконок
- [Визуальные темы](../base/theme.md)
