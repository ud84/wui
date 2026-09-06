# Graphics

`graphic` wraps the platform rendering context. Bounds are passed by value.

```cpp
graphic(system_context &context);
    ~graphic();

    bool init(rect max_size, color background_color);
    void release();

    rect max_size() const;

    void set_background_color(color background_color);

    void clear(rect position = { 0 });

    void flush(rect updated_size);

    void draw_pixel(rect position, color color_);

    void draw_line(rect position, color color_, uint32_t width = 1);

    rect measure_text(std::string_view text_, const font &font__);
    void draw_text(rect position, std::string_view text, color color_, const font &font_);

    void draw_rect(rect position, color fill_color);
    void draw_rect(rect position, color border_color, color fill_color, uint32_t border_width, uint32_t round);

    /// Draws a PNG scaled into position. The loaded surface is cached per
    /// path and owned by this graphic (released on destruction).
    /// Implemented on Linux only (no-op elsewhere). The cache is not
    /// invalidated: if the file changes on disk, the previous image is
    /// reused until the graphic is destroyed.
    void draw_image(std::string_view file_name, rect position);

    /// draw some buffer on context
    void draw_buffer(rect position, uint8_t *buffer, int32_t left_shift, int32_t top_shift);

    /// draw another graphic on context; position is {x, y, width, height}
    void draw_graphic(rect position, graphic &graphic_, int32_t left_shift, int32_t top_shift);
```

`init()` returns success. `draw_text()` requires bounds, text, color and font.
Use `wui::image` for portable images: `graphic::draw_image()` is currently Linux-only.

Most rectangles use `{left, top, right, bottom}`. `draw_graphic()` is a legacy
exception: its destination rectangle is interpreted as `{x, y, width, height}`;
`left_shift` and `top_shift` select the source offset. Do not reuse control bounds
without converting their right/bottom edges into extents.

macOS uses logical points and WASM uses CSS pixels; their backing buffers account
for display scale. In a panel callback, coordinates belong to the window, not to
a separate panel-local canvas.
