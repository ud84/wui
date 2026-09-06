# Basic interfaces

## Window

The current `i_window` interface uses `std::string_view` and by-value `rect`.

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

`add_control()` retains a control; `remove_control()` detaches it. Keep another
shared reference if it will be reused. `subscribe()` returns an ID for later
unsubscription. `window` adds methods for focus, themes, modal relationships and
user events. `set_min_size()` constrains resizing.

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

Set geometry with `set_position(rect)`. There is no second redraw flag; invalidate
the relevant window area when moving multiple controls. Parent references are weak.
See [ownership](ownership.md), [geometry](common.md) and [events](event.md).
