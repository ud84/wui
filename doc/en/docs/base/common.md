# Common structures and data types

Below are the common structures and types used in the framework

## color

A type for storing color, defined as follows:
```typedef unsigned long color;```

Has the following helpers:

- ```color make_color(unsigned char red, unsigned char green, unsigned char blue)```
- ```unsigned char get_red(color rgb)```
- ```unsigned char get_green(color rgb)```
- ```unsigned char get_blue(color rgb)```

## font

Structure containing font parameters

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

- ```name``` - name of the system font, e.g. sans
- ```size`` - the size of the font in pixels
- ```decorations`` - bitmask defining font layout variants

## rect

Structure, to transfer coordinates and simplify work with them

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

- ```left, top, right, bottom`` - coordinates relative to the upper left corner of the window
- ```operator==, operator>, operator+``` - operators
- ```in(int32_t x, int32_t y)``` - determines whether a point is included in the coordinate area or not.
- ```in(const rect &outer)`` - determines whether the coordinates are included in the outer coordinate area
- ```is_null()``` - determines if the coordinates are set (true if all are null)
- ```width(), height()``` - width and height
- ```move(int32_t x, int32_t y)``` - shifts coordinates by x and y
- ```put(int32_t x, int32_t y)``` - sets left and top to x and y

