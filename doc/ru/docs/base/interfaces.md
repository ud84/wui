# Базовые интерфейсы

## Window

Актуальный `i_window` использует `std::string_view` и передачу `rect` по значению.

```cpp
class i_window
{
public:
    virtual bool init(std::string_view caption, rect position, window_style style, std::function<void(void)> close_callback) = 0;
    virtual void destroy() = 0;

    virtual void add_control(std::shared_ptr<i_control> control, rect position) = 0;
    virtual void remove_control(std::shared_ptr<i_control> control) = 0;

    virtual void bring_to_front(std::shared_ptr<i_control> control) = 0;
    virtual void move_to_back(std::shared_ptr<i_control> control) = 0;

    virtual void redraw(rect position, bool clear = false) = 0;

    virtual std::string subscribe(std::function<void(const event&)> receive_callback, event_type event_types, std::shared_ptr<i_control> control = nullptr) = 0;
    virtual void unsubscribe(std::string_view subscriber_id) = 0;

    virtual system_context &context() = 0;

    virtual error get_error() const = 0;

protected:
    ~i_window() {}

};
```

`add_control()` удерживает контрол, `remove_control()` отсоединяет его. Для
повторного использования сохраните свою shared-ссылку. `subscribe()` возвращает
ID для отписки. Класс `window` добавляет фокус, темы, модальные связи и пользовательские
события. `set_min_size()` ограничивает размер при растягивании окна.

## Control

```cpp
class i_control
{
public:
    virtual void draw(graphic &gr, rect paint_rect) = 0;

    virtual void set_position(rect position) = 0;
    virtual rect position() const = 0;

    virtual void set_parent(std::shared_ptr<window> window_) = 0;
    virtual std::weak_ptr<window> parent() const = 0;
    virtual void clear_parent() = 0;

    virtual void set_topmost(bool yes) = 0;
    virtual bool topmost() const = 0;

    virtual void update_theme_control_name(std::string_view theme_control_name) = 0;
    virtual void update_theme(std::shared_ptr<i_theme> theme_ = nullptr) = 0;

    virtual void show() = 0;
    virtual void hide() = 0;
    virtual bool showed() const = 0;

    virtual void enable() = 0;
    virtual void disable() = 0;
    virtual bool enabled() const = 0;

    virtual bool focused() const = 0;  /// Returns true if the control is focused
    virtual bool focusing() const = 0; /// Returns true if the control receives focus

    virtual error get_error() const = 0;

    friend class window;

protected:
    ~i_control() {}

};
```

Геометрия задаётся `set_position(rect)`, второго флага перерисовки нет. При
перемещении нескольких контролов запросите перерисовку соответствующей области
окна. Ссылки на родителя слабые. См. [владение](ownership.md), [координаты](common.md), [события](event.md).
