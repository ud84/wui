# Graphical context

Graphical context is an encapsulation of the system graphical context
and serves to unify rendering procedures and isolate platform-dependent code.

The interface:

	class graphic
	{
	public:
		graphic(system_context &context);

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
    	void draw_buffer(const rect &position, uint8_t *buffer, int32_t left_shift, int32_t top_shift);

    	/// draw another graphic on context
    	void draw_graphic(const rect &position, graphic &graphic_, int32_t left_shift, int32_t top_shift);
	};

## constructor
Takes the system context from the window as input

## init
Initialization of the subsystem

- max_size - maximum size of the drawing field
- background_color - fill color

## release
Deinitialization of the subsystem, clearing

## set_background_color
Change fill color

## clear
Clearing the area with fill color

- position - coordinates of the area to be cleared

## flush
Resetting (drawing) the area to the system graphic context

- updated_size - coordinates of the reset area

## draw_pixel
Drawing a point

- position - coordinates of the point
- color_ - color of the point

## draw_line
Drawing a line

- position - coordinates of the start and end points
- color_ - line color
- width - line thickness

## measure_text
Returns the dimensions of the text line in pixels

- text - line
- font_ - font

## draw_text
Draw text string

- position - coordinates of the upper left corner of the string
- text - line text
- color_ - color
- font_ - font

## draw_rect #1
Drawing a simple rectangle

- position - coordinates
- fill_color - fill color

## draw_rect #2
Drawing a rectangle with border and rounding

- position - coordinates
- border_color - border color
- fill_color - fill color
- border_width - border thickness
- round - radius of rounding

## draw_buffer
Draw buffer with RGB32 color depth

- position - coordinates for drawing
- buffer - buffer
- buffer_size - buffer size in bytes

## draw_graphic
Drawing another graphic context

- position - coordinates for drawing
- graphic_ - graphic context
- left_shift - x offset
- top_shift - y offset
