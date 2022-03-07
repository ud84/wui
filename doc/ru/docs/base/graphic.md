# Графический контекст

Графический контекст представляет собой инкапсуляцию системного графического контекста
и служит для унификации процедур отрисовки и изоляции платформозависимого кода.

Интерфейс:

	class graphic
	{
	public:
		graphic(system_context &context);
		~graphic();

		void init(const rect &max_size, color background_color);
		void release();

		void set_background_color(color background_color);

		void clear(const rect &position);

		void flush(const rect &updated_size);

		void draw_pixel(const rect &position, color color_);

		void draw_line(const rect &position, color color_, uint32_t width = 1);

		rect measure_text(const std::string &text, const font &font_);
		void draw_text(const rect &position, const std::string &text, color color_, const font &font_);

		void draw_rect(const rect &position, color fill_color);
		void draw_rect(const rect &position, color border_color, color fill_color, uint32_t border_width, uint32_t round);

		/// draw some buffer on context
		void draw_buffer(const rect &position, uint8_t *buffer, size_t buffer_size);

		/// draw another graphic on context
		void draw_graphic(const rect &position, graphic &graphic_, int32_t left_shift, int32_t right_shift);
	};

## constructor
Принимает на вход системный контекст от окна

## init
Инициализация подсистемы

- max_size - максимальный размер поля рисования
- background_color - цвет заливки

## release
Денинициализация подсистемы, очистка

## set_background_color
Смена цвета заливки

## clear
Очистка области цветом заливки

- position - координаты очищаемой области

## flush
Сброс (отрисовка) области на системный графический контекст

- updated_size - координаты сбрасываемой области

## draw_pixel
Рисование точки

- position - координаты точки
- color_ - цвет точки

## draw_line
Рисование линии

- position - координаты начальной и конечной точек
- color_ - цвет линии
- width - толщина линии

## measure_text
Возвращает размеры текстовой строки в пикселях

- text - строка
- font_ - шрифт

## draw_text
Отрисовка текстовой строки

- position - координаты верхнего левого угла строки
- text - текст строки
- color_ - цвет
- font_ - шрифт

## draw_rect #1
Отрисовка простого прямоугольника

- position - координаты
- fill_color - цвет заливки

## draw_rect #2
Отрисовка прямоугольника с границей и скруглением

- position - координаты
- border_color - цвет границы
- fill_color - цвет заливки
- border_width-толщина границы
- round - радиус скругления

## draw_buffer
Отрисовка буфера с глубиной цвета RGB32

- position - координаты для отрисовки
- buffer - буфер
- buffer_size - размер буфера в байтах

## draw_graphic
Отрисовка другого графического контекста

- position - координаты для отрисовки
- graphic_ - графический контекст
- left_shift - смещение по x
- top_shift - смещение по y
