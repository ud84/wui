# Common structures

Ниже представлены общие структуры и типы используемые в фреймворке

## color

Тип для хранения цвета, определяется так:
```typedef unsigned long color;```

Имеет следующие хелперы:

- ```color make_color(unsigned char red, unsigned char green, unsigned char blue)```
- ```unsigned char get_red(color rgb)```
- ```unsigned char get_green(color rgb)```
- ```unsigned char get_blue(color rgb)```

## font

Структура, содержащая параметры шрифта

	enum class decorations : uint32_t
	{
		normal = 0,
		bold = (1 << 0),
		italic = (1 << 1),
		underline = (1 << 2),
		strike_out = (1 << 3)
	};

	struct font
	{
		std::string name;
		int32_t size;
		decorations decorations_;
	};

- ```name``` - наименование системного шрифта, например sans
- ```size``` - размер кегля в пикселах
- ```decorations``` - битовая маска определяюшая варианты начертания шрифта

## rect

Структура, для передачи координат и упрощения работы с ними

	struct rect
	{
		int32_t left, top, right, bottom;

		inline bool operator==(const rect &lv);
        inline bool operator>(const rect &lv);
        inline rect operator+(const rect &lv);

		inline bool in(int32_t x, int32_t y) const;
		inline bool in(const rect &outer) const;

		inline bool is_null() const;

		inline int32_t width() const;
		inline int32_t height() const;

        inline void move(int32_t x, int32_t y);
        inline void put(int32_t x, int32_t y);
	};

- ```left, top, right, bottom``` - координаты, относительно верхнего левого угла окна
- ```operator==, operator>, operator+``` - операторы
- ```in(int32_t x, int32_t y)``` - определяет входит ли точка в область координат
- ```in(const rect &outer)``` - определяет входят ли координаты в область координат outer
- ```is_null()``` - определяет заданы ли координаты (true если все равны null)
- ```width(), height()``` - ширина и высота
- ```move(int32_t x, int32_t y)``` - смещает координаты на x и y
- ```put(int32_t x, int32_t y)``` - устанавливает left и top в x и y
