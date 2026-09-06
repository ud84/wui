# Выпадающий список (select)

Контрол `select` предоставляет выпадающий список для выбора одного значения из множества.

## Быстрый старт

```cpp
#include <wui/control/select.hpp>

auto select = std::make_shared<wui::select>();

// Элементы списка
wui::select_items_t items = {
    {1, "Option 1"},
    {2, "Option 2"},
    {3, "Option 3"}
};

select->set_items(items);
select->select_item_number(0); // Выбрать первый

select->set_change_callback([](int32_t index, int64_t id) {
    std::cout << "Selected: " << index << " (ID: " << id << ")" << std::endl;
});

window->add_control(select, {10, 10, 150, 30});
```

## Структура элемента

![Селектор WUI](../img/select.png)

```cpp
struct select_item
{
    int64_t id;          // Уникальный ID
    std::string text;    // Текст отображения
};
```

## API

### Конструктор

```cpp
select(std::string_view theme_control_name = tc, 
       std::shared_ptr<i_theme> theme_ = nullptr);
```

### Методы

```cpp
// Управление элементами
void set_items(const select_items_t &items);
void update_item(const select_item &mi);
void swap_items(int64_t first_item_id, int64_t second_item_id);
void delete_item(int64_t id);

// Выбор
void select_item_number(int32_t index);
void select_item_id(int64_t id);
select_item selected_item() const;
const select_items_t &items() const;

// Высота
void set_item_height(int32_t item_height);

// Коллбэк
void set_change_callback(std::function<void(int32_t, int64_t)> change_callback);
```

## Примеры

### Выбор языка

```cpp
auto language_select = std::make_shared<wui::select>();

wui::select_items_t languages = {
    {1, "English"},
    {2, "Русский"},
    {3, "Deutsch"},
    {4, "Français"},
    {5, "Español"}
};

language_select->set_items(languages);
language_select->select_item_id(2); // Русский по умолчанию

language_select->set_change_callback([](int32_t index, int64_t id) {
    switch (id) {
        case 1: set_language("en"); break;
        case 2: set_language("ru"); break;
        case 3: set_language("de"); break;
        case 4: set_language("fr"); break;
        case 5: set_language("es"); break;
    }
});

window->add_control(language_select, {10, 10, 150, 30});
```

### Выбор размера шрифта

```cpp
auto font_size_select = std::make_shared<wui::select>();

wui::select_items_t sizes = {
    {8, "8 pt"},
    {10, "10 pt"},
    {12, "12 pt"},
    {14, "14 pt"},
    {16, "16 pt"},
    {18, "18 pt"},
    {24, "24 pt"}
};

font_size_select->set_items(sizes);
font_size_select->select_item_id(12);

font_size_select->set_change_callback([](int32_t, int64_t id) {
    set_font_size(static_cast<int>(id));
});

window->add_control(font_size_select, {10, 50, 100, 80});
```

### Динамическое обновление

```cpp
auto country_select = std::make_shared<wui::select>();
auto city_select = std::make_shared<wui::select>();

// Страны
wui::select_items_t countries = {
    {1, "Russia"},
    {2, "USA"},
    {3, "Germany"}
};

country_select->set_items(countries);

country_select->set_change_callback(
    [&city_select](int32_t, int64_t country_id) {
        // Обновляем города при смене страны
        wui::select_items_t cities;
        
        switch (country_id) {
            case 1:
                cities = {{101, "Moscow"}, {102, "St. Petersburg"}};
                break;
            case 2:
                cities = {{201, "New York"}, {202, "Los Angeles"}};
                break;
            case 3:
                cities = {{301, "Berlin"}, {302, "Munich"}};
                break;
        }
        
        city_select->set_items(cities);
        city_select->select_item_number(0);
    }
);

window->add_control(country_select, {10, 10, 150, 30});
window->add_control(city_select, {10, 50, 150, 80});
```

### Удаление элемента

```cpp
auto removable_select = std::make_shared<wui::select>();

wui::select_items_t items = {
    {1, "Item 1"},
    {2, "Item 2"},
    {3, "Item 3"}
};

removable_select->set_items(items);

// Кнопка для удаления выбранного
auto remove_btn = std::make_shared<wui::button>("Remove", [&]() {
    auto selected = removable_select->selected_item();
    if (selected.id != 0) {
        removable_select->delete_item(selected.id);
    }
});

window->add_control(removable_select, {10, 10, 150, 30});
window->add_control(remove_btn, {170, 10, 250, 30});
```

## Темизация

```json
{
  "type": "select",
  "background": "#ffffff",
  "border": "#cccccc",
  "border_width": 1,
  "hover_border": "#999999",
  "focused_border": "#0078d7",
  "button_calm": "#f0f0f0",
  "button_active": "#e0e0e0",
  "text": "#000000",
  "scrollbar": "scrollbar",
  "scrollbar_slider": "scrollbar_slider",
  "scrollbar_slider_active": "scrollbar_slider_active",
  "selected_item": "#cce8ff",
  "active_item": "#e5f3ff",
  "round": 4,
  "font": {"name": "Segoe UI", "size": 18}
}
```

## См. также

- [Список](list.md) — использует list internally
- [Кнопка](button.md) — для кнопки раскрытия
