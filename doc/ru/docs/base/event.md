# События

## Модель подписки на события (Event Subscription Model)

В libwui реализована модель **«плоской подписки»**. В отличие от традиционных фреймворков, где события передаются по цепочке наследования, здесь **Окно (Window)** выступает в роли центрального диспетчера. Любой объект — будь то контрол, диалог или бизнес-логика — может подписаться на поток событий окна.

### Преимущества

**Свобода от иерархии (Decoupling)**
Вам не нужно наследоваться от `wui::button`, чтобы переопределить `on_click`. Вы просто подписываете любой метод любого класса на события нужного типа. Это позволяет разделять UI и логику (реальный MVVM/MVC без боли).

**Таргетированные или глобальные события**
- Если параметр `control` передан — вы получаете события только для этого контрола (клики, фокус).
- Если `control == nullptr` — вы подписываетесь на системные или внутренние события окна (ресайз, смена темы, горячие клавиши). Это идеальное место для логики адаптивного лейаута.

**Управление временем жизни (Subscriber ID)**
Метод возвращает уникальный `subscriber_id` (строку). Это позволяет динамически отписываться (`unsubscribe`) в любой момент, не разрушая объект. Это критично для «открепляемых» окон и динамических интерфейсов.

**Битовая маска типов событий**
Параметр `event_type` — это битовая маска. Можно подписаться сразу на `mouse | keyboard | internal`, что экономит количество регистраций коллбэков.

```cpp
// Подписываемся на события клавиатуры и системные изменения
std::string sub_id = window->subscribe([this](const wui::event& ev) {
    if (ev.type == wui::event_type::keyboard) {
        // Логика горячих клавиш
    }
}, wui::event_type::keyboard | wui::event_type::system);

// Когда компонент уничтожается - просто отписываемся по ID
window->unsubscribe(sub_id);
```

### Пример использования

```cpp
// MainFrame подписывается на ресайз окна
window->subscribe([this](const wui::event& ev) {
    if (ev.type == wui::event_type::internal && 
        ev.internal_event_.type == wui::internal_event_type::size_changed) {
        // Адаптивный лейаут
        updateLayout(ev.internal_event_.x, ev.internal_event_.y);
    }
}, wui::event_type::internal);

// DialingDialog подписывается на кнопку отмены
cancelButton = std::make_shared<wui::button>("Cancel", [this]() {
    Cancel();
});
```

> **Примечание:** Мы отказались от концепции виртуальных обработчиков `on_something` в пользу единой шины событий окна. Теперь любой компонент — это просто подписчик. Хотите, чтобы боковая панель знала о нажатии клавиши в текстовом поле? Просто подпишите её. Это превращает монолитный UI в гибкий конструктор, где компоненты общаются через `std::function`, не зная ничего друг о друге.

## Базовый API

Для обработки событий используется подписная модель. Контрол или пользовательский код подписывается в окне на нужные ему события и получает их.
События могут быть сгенерированы системой или пользовательским кодом.

Все события проходят через окно. Для работы с ними используются следующие методы:

	class window
	{
	public:
		...
		std::string subscribe(std::function<void(const event&)> receive_callback, event_type event_types, std::shared_ptr<i_control> control = nullptr);
		void unsubscribe(std::string_view subscriber_id);

		void emit_event(int32_t x, int32_t y);
		...
	};

### std::string subscribe(...)
Используется для получения событий от окна. Метод возвращает уникальный идентификатор подписчика 
который нужно передать в ```unsubscribe()``` для отписки.

- receive_callback - функция, получающая события
- event_types - битовая маска, указывающая какие события нужно получать
- control - при установке данного поля будут приходить только события мыши происходящие над этим контролом
и клавиатурные события если у него имеется фокус ввода. В противном случае, будут приходить все события от окна.

### unsubscribe(std::string_view subscriber_id)
Предназначен для прекращения подписки на события окна 

- subscriber_id - уникальный идентификатор подписчика, выданный методом ```subscribe```

### emit_event(int32_t x, int32_t y)
Метод, генерирующий internal event. Используется пользовательским кодом для генерации собственных событий.

## Типы событий

	enum class event_type : uint32_t
	{
		system = (1 << 0),
		mouse = (1 << 1),
		keyboard = (1 << 2),
		internal = (1 << 3),

		all = system | mouse | keyboard | internal
	};

	struct event
	{
		event_type type;

		union
		{
			mouse_event mouse_event_;
			keyboard_event keyboard_event_;
			internal_event internal_event_;
		};
	};

В зависимости от типа события берется значение из объединения event.

## События мыши

	enum class mouse_event_type
	{
		move,
		enter,
		leave,
		right_down,
		right_up,
		center_down,
		center_up,
		left_down,
		left_up,
		left_double,
		wheel
	};

	struct mouse_event
	{
		mouse_event_type type;

		int32_t x, y;

		int32_t wheel_delta;
	};

## События клавиатуры

	enum class keyboard_event_type
	{
		down, 
		up,
		key
	};

	struct keyboard_event
	{
		keyboard_event_type type;

		uint8_t modifier; /// vk_capital or vk_shift or vk_alt or vk_insert
		char key[4];
		uint8_t key_size;
	};

## Внутренние события

	enum class internal_event_type
	{
		set_focus,
		remove_focus,
		execute_focused,
    
		size_changed,
		position_changed,

		user_emitted
	};

	struct internal_event
	{
		internal_event_type type;
		int32_t x, y;
	};
