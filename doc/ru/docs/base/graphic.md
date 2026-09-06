# Графика

`graphic` скрывает платформенный контекст отрисовки. Границы передаются по значению.

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

`init()` возвращает признак успеха. `draw_text()` принимает границы, текст, цвет и шрифт.
Для переносимых изображений используйте `wui::image`: `graphic::draw_image()` пока работает только на Linux.

Большинство прямоугольников — `{left, top, right, bottom}`. Историческое исключение —
`draw_graphic()`: его назначение задаётся как `{x, y, width, height}`, а
`left_shift`/`top_shift` задают смещение источника. Преобразуйте правую и нижнюю
границы контрола в ширину и высоту перед таким вызовом.

macOS использует логические точки, WASM — CSS-пиксели; буферы учитывают масштаб
экрана. В callback панели координаты относятся к окну, а не к отдельному canvas панели.
