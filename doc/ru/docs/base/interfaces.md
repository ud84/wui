# Базовые интерфейсы 

Фреймворк основан на нескольких базовых интерфейсах, таких как i_window, i_control.
Данные интерфейсы содержат методы которые обязательно должны быть реализованы окном (в случае i_window) и всеми контролами (в случае i_control). Но каждый контрол (а также реализация окна) также имеет дополнительные специфичные методы.

# Window
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

## bool init(const std::string &caption, const rect &position, window_style style, std::function<void(void)> close_callback)
Первоначальное создание окна. Данный метод должен быть вызван для создания отдельного (базового) окна, а также может использоваться для установки параметров дочернего окна.

- caption - заголовок окна
- position - размеры и положение окна. Если left установлен в -1 окно центрируется по горизонтали, если top установлен в 1 окно центрируется по вертикали
- style - комбинация битовых масок определяющих внешний вид окна
- close_callback - функция вызываемая при закрытии окна пользователем, вызове ```destroy()``` или откреплении дочернего окна от базового (вызовом ```remove_control()```)

- Возвращает true если окно создано успешно

## void destroy()
Вызывается когда окно нужно уничтожить

## void add_control(std::shared_ptr<i_control> control, const rect &position)
Добавляет на окно дочерний контрол (в том числе дочернее окно)

- control - указатель на добавляемый контрол
- position - положение контрола на окне

## void remove_control(std::shared_ptr<i_control> control)
Удаляет дочерний контрол с окна. Если дочерним контролом явлется окно, оно становится независимым (базовым)

- control - указатель на удаляемый контрол

## void redraw(const rect &position, bool clear = false)
Запускает перерисовку части окна с имеющимися на данном участке контролами.

- position - границы перерисовываемой области
- clear нужно устанавливать например при удалении или перемещении контрола

## std::string  subscribe(std::function<void(const event&)> receive_callback, event_type event_types, std::shared_ptr<i_control> control = nullptr)
Используется для получения событий от окна. Метод возвращает уникальный идентификатор подписчика который нужно передать в ```unsubscribe()``` для отписки.

- receive_callback - функция, получающая события
- event_types - битовая маска, указывающая какие события нужно получать
- control - при установке данного поля будут приходить только события мыши происходящие над этим контролом и клавиатурные события если у него имеется фокус ввода.
В противном случае, будут приходить все события от окна.

## void unsubscribe(const std::string &subscriber_id)
Предназначен для прекращения подписки на события окна

- subscriber_id - уникальный идентификатор подписчика, выданный методом ```subscribe```

## system_context &context()
Возвращает ссылку на структуру, содержащую платформозависимые сущности. Например дескриптор окна HWND в Windows или xcb_connection в Linux.

# Control
Данный интерфейс должны реализовывать все контролы, а также окно, так как оно тоже может являеться контролом (дочерним окном)

	class i_control
	{
	public:
		virtual void draw(graphic &gr, const rect &paint_rect) = 0;

		virtual void set_position(const rect &position, bool redraw = true) = 0;
		virtual rect position() const = 0;

		virtual void set_parent(std::shared_ptr<window> window_) = 0;
		virtual std::weak_ptr<window> parent() const = 0;
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

## void draw(graphic &gr, const rect &paint_rect)
Метод вызывается окном при необходимости перерисовать контрол (например при вызове window::redraw)

- gr - ссылка на графический контекст окна, на котором контрол должен себя нарисовать
- paint_rect - область которая должна быть перерисована. Обычно игнорируется простыми контролами

## void set_position(const rect &position, bool redraw = true)
Мeтод для изменения положения контрола на окне. Координаты передаются в этот метод относительно родительского окна контрола.

- position - новое положение контрола
- redraw - указывает окну, нужно ли очищать предыдущее положение контрола. 
Для улучшения производительности, должно быть установлено в false при вызове ```set_position``` в ответ на изменение размеров окна (так как в этом случае очищается все окно)

## rect position()
Возвращает координаты контрола относительно базового окна. Т.е. если контрол находится на дочернем окне, ```position()``` должен вернуть положение 
контрола на базовом(физическом) окне.

## void set_parent(std::shared_ptr<window> window_)
Метод, вызываемый родительским окном при вызове ```add_control()```, позволяет контролу получить указатель на свое родителькое окно.

## std::weak_ptr<window> parent() const
Возвращает указатель на содержащее контрол окно

## void clear_parent()
Метод, вызываемый родительским окном при вызове ```remove_control()``` очищает указатель на родителькое окно контрола

## bool topmost()
Сообщает родительскому окну, нужно ли рисовать контрол поверх всех остальных контролов

## void update_theme(std::shared_ptr<i_theme> theme_ = nullptr)
Изменяет визуальную тему контрола. Если параметр равен nullptr то используется тема приложения по умолчанию.

## void show()
Включает отображение контрола (если оно было выключено методом ```hide()```)

## void hide()
Выключает отображение контрола

## bool showed() const
Возвращает состояние отображения контрола (включено или выключено)

## void enable()
Включает контрол (если он был выключен методом ```disable()```)

## void disable()
Выключает контрол. Выключенный контрол, как правило, не реагирует на пользовательские события, не принимает фокус ввода и отрисовывается серым цветом

## bool enabled() const
Возвращает состояние контрола: включен/выключен

## bool focused() const
Возвращает наличие фокуса ввода у контрола

## bool focusing() const
Возвращает принимает ли контрол фокус ввода. Выключеный контрол, контрол не взаимодействующий с пользователем возвращает false
