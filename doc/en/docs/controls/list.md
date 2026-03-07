# List

The `list` control provides a list of items with support for columns and scrolling.

## Quick Start

```cpp
#include <wui/control/list.hpp>

auto list = std::make_shared<wui::list>();

// Setup columns
list->update_columns({
    {100, "Name"},
    {150, "Date"},
    {80, "Size"}
});

// Set item count
list->set_item_count(100);

// Draw callback
list->set_draw_callback([](wui::graphic& gr, int32_t n_item, 
                            wui::rect item_rect, wui::list::item_state state) {
    gr.draw_text(item_rect, get_item_text(n_item));
});

window->add_control(list, {10, 10, 400, 300});
```

## List Modes

```cpp
enum class list_mode {
    simple, auto_select, simple_topmost
};
```

## Item States

```cpp
enum class item_state {
    normal, active, selected
};
```

![WUI List](../img/list0.png)

![WUI List with data](../img/list1.png)

## API

```cpp
// Columns
void update_columns(const std::vector<column> &columns_);
void set_column_width(int32_t n_column, int32_t width);

// Items
void set_item_count(int32_t count);
int32_t get_item_count() const;
void select_item(int32_t n_item);
int32_t selected_item() const;

// Callbacks
void set_draw_callback(std::function<void(graphic&, int32_t, rect, item_state)> cb);
void set_item_click_callback(std::function<void(click_button, int32_t, int32_t, int32_t)> cb);
void set_item_change_callback(std::function<void(int32_t)> cb);
void set_column_click_callback(std::function<void(int32_t)> cb);
```

## See Also

- [Select](select.md) — dropdown list
- [Scroll](scroll.md) — scrollbar
