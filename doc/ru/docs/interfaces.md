# Базовые интерфейсы 

Фреймворк основан на нескольких базовых интерфейсах, таких как i_window, i_control.
Данные интерфейсы содержат методы которые обязательно должны быть реализованы окном (в случае i_window) и всеми контролами (в случае i_control). Но каждый контрол (а также реализация окна) также имеет дополнительные специфичные методы.

## Интерфейс Window
Этот интерфейс реализовывается окном

	class i_window
	{
	public:
		virtual bool init(const std::string &caption, const rect &position, window_style style, std::function<void(void)> close_callback) = 0;
		virtual void destroy() = 0;

		virtual void add_control(std::shared_ptr<i_control> control, const rect &position) = 0;
		virtual void remove_control(std::shared_ptr<i_control> control) = 0;

		virtual void redraw(const rect &position, bool clear = false) = 0;

		virtual std::string subscribe(std::function<void(const event&)> receive_callback, event_type event_types, std::shared_ptr<i_control> control = nullptr) = 0;
		virtual void unsubscribe(const std::string &subscriber_id) = 0;

		virtual system_context &context() = 0;

	protected:
		~i_window() {}

	};

### init
```init(const std::string &caption, const rect &position, window_style style, std::function<void(void)> close_callback)``` служит для первоначального создания окна.
Данный метод должен быть вызван для создания отдельного (базового) окна, а также может использоваться для установки параметров дочернего окна.

- caption - заголовок окна
- position - размеры и положение окна. Если left установлен в -1 окно центрируется по горизонтали, если top установлен в -1 окно центрируется по вертикали
- style - комбинация битовых масок определяющих внешний вид окна
- close_callback - функция вызываемая при закрытии окна пользователем, вызове ```destroy()``` или откреплении дочернего окна от базового (вызовом ```remove_control()```)

### destroy
```destroy()``` вызывается когда окно нужно уничтожить

### add_control
```add_control(std::shared_ptr<i_control> control, const rect &position)``` добавляет на окно дочерний контрол (в том числе дочернее окно)

- control - указатель на добавляемый контрол
- position - положение контрола на окне

### remove_control
```remove_control(std::shared_ptr<i_control> control)``` удаляет дочерний контрол с окна. Если дочерним контролом явлется окно, оно становится независимым (базовым)

- control - указатель на удаляемый контрол

### redraw
```redraw(const rect &position, bool clear = false)``` запускает перерисовку части окна с имеющимися на данном участке контролами.

- position - границы перерисовываемой области
- clear нужно устанавливать например при удалении или перемещении контрола

### subscribe
```std::string subscribe(std::function<void(const event&)> receive_callback, event_type event_types, std::shared_ptr<i_control> control = nullptr)``` используется для получения 
событий от окна. Метод возвращает уникальный идентификатор подписчика который нужно передать в ```unsubscribe()``` для отписки.

- receive_callback - функция, получающая события
- event_types - битовая маска, указывающая какие события нужно получать
- control - при установке данного поля будут приходить только события происходящие надо этим контролом (или если у него имеется фокус ввода).
В противном случае, будут приходить все события от окна.

### unsubscribe
```unsubscribe(const std::string &subscriber_id)``` предназначен для прекращения подписки на события окна
- subscriber_id - уникальный идентификатор подписчика, вдыанный методом ```subscribe```

### system_context
Метод ```system_context()``` возвращает ссылку на структуру, содержащую платформозависимые сущности. Например дескриптор окна HWND в Windows или xcb_connection в Linux.

## Интерфейс Control
Данный интерфейс должны реализовывать все контролы, а также окно, так как оно тоже может являеться контролом (дочерним окном)

	class i_control
	{
	public:
		virtual void draw(graphic &gr, const rect &paint_rect) = 0;

		virtual void set_position(const rect &position, bool redraw = true) = 0;
		virtual rect position() const = 0;

		virtual void set_parent(std::shared_ptr<window> window_) = 0;
		virtual void clear_parent() = 0;

		virtual bool topmost() const = 0;

		virtual void update_theme(std::shared_ptr<i_theme> theme_ = nullptr) = 0;

		virtual void show() = 0;
		virtual void hide() = 0;
		virtual bool showed() const = 0;

		virtual void enable() = 0;
		virtual void disable() = 0;
		virtual bool enabled() const = 0;

		virtual bool focused() const = 0;  /// Returns true if the control is focused
		virtual bool focusing() const = 0; /// Returns true if the control receives focus

		friend class window;

	protected:
		~i_control() {}

	};

### draw
```draw(graphic &gr, const rect &paint_rect)``` метод вызывается окном при необходимости перерисовать контрол (например при вызове window::redraw)

- gr - ссылка на графический контекст окна, на котором контрол должен себя нарисовать
- paint_rect - область которая должна быть перерисована. Обычно игнорируется простыми контролами

### set_position
```set_position(const rect &position, bool redraw = true)``` метод для изменения положения контрола на окне

- position - новое положение контрола
- redraw - указывает окну, нужно ли очищать предыдущее положение контрола. 
Для улучшения производительности, должно быть установлено в false при вызове ```set_position``` в ответ на изменение размеров окна (так как в этом случае очищается все окно)
