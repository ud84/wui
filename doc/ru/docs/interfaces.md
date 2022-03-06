# Базовые интерфейсы 

Фреймворк основан на нескольких базовых интерфейсах, таких как Window, Control, Theme, Event и Locale

## Интерфейс Window

	class i_window
	{
	public:
		virtual bool init(const std::string &caption, const rect &position, window_style style, std::function<void(void)> close_callback, std::shared_ptr<i_theme> theme_ = nullptr) = 0;
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

* ```init()``` служит для первоначального создания окна. Данный метод должен быть вызван для создания как отдельного (базового) окна, так и для инциализации окна являющегося дочерним.
	- caption - заголовок окна
	- position - размеры и положение окна. Если left установлен в -1 окно центрируется по горизонтали, если top установлен в -1 окно центрируется по вертикали
	- style - комбинация битовых масок определяющих внешний вид окна
	- close_callback - функция вызываемая при закрытии окна пользователем, вызове ```destroy()``` или откреплении дочернего окна от базового (вызовом ```remove_control()```)
	- theme - указатель на персональную 

* ```destroy()``` вызывается когда окно нужно уничтожить

* ```add_control()``` добавляет на окно дочерний контрол (в том числе дочернее окно)

* ```remove_control()``` удаляет дочерний контрол с окна. Если дочерним контролом явлется окно, оно становится независимым (базовым)

* ```redraw()``` запускает перерисовку части окна с имеющимися на данном участке контролами. Флаг clear нужно устанвливать например при удалении или перемещении контрола.
