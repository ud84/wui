# Visual Themes

The Theme subsystem provides a convenient way to centrally store visual settings.
The subsystem is designed to display application controls in a single view and easily centralize the ability to change the current theme on the fly.

The subsystem provides handy helpers for working with themes. These helpers operate with a singleton that 
all application controls use by default. The subsystem allows you to load a theme from json, which can be stored on disk or be a Windows application resource.

Example [json file](https://github.com/ud84/wui/blob/main/res/dark.json) containing visual parameters of the main controllers.

Although the application always has a default theme, you can create your own custom theme and specify any control to use it. In this case, the control will be displayed according to your theme.

Below are the main helpers for centralizing the application theme. These helpers essentially represent
access to the default theme instance singleton of the application.

### Helpers

    namespace wui
    {

    /// Set and get the current theme
    #ifdef _WIN32
    bool set_default_theme_from_resource(const std::string &name, int32_t resource_index, const std::string &resource_section);
    #endif
    bool set_default_theme_from_json(const std::string &name, const std::string &json);
    bool set_default_theme_from_file(const std::string &name, const std::string &file_name);
    void set_default_theme_empty(const std::string &name);

    /// Return the pointer to current default theme instance
    std::shared_ptr<i_theme> get_default_theme();

    /// Make the custom theme for the some control
    std::shared_ptr<i_theme> make_custom_theme(const std::string &name = "");
    std::shared_ptr<i_theme> make_custom_theme(const std::string &name, const std::string &json);

    /// Return the item's color by current theme
    color theme_color(const std::string &control, const std::string &value, std::shared_ptr<i_theme> theme_ = nullptr);

    /// Return the item's dimension by current theme
    int32_t theme_dimension(const std::string &control, const std::string &value, std::shared_ptr<i_theme> theme_ = nullptr);

    /// Return the item's string value by current theme
    const std::string &theme_string(const std::string &control, const std::string &value, std::shared_ptr<i_theme> theme_ = nullptr);

    /// Return the item's font value by current theme
    font theme_font(const std::string &control, const std::string &value, std::shared_ptr<i_theme> theme_ = nullptr);

    const std::vector<uint8_t> &theme_image(const std::string &name, std::shared_ptr<i_theme> theme_ = nullptr);

### set_default_theme_from_resource (Windows only)
#### Input parameters
 - const std::string &name - theme name, e.g. "dark"
 - int32_t resource_index - ID ресурса
 - const std::string &resource_section - resource section, e.g. "JSONS"
#### Return value
 - true if successful

Function that loads the default application theme from a json file stored in the application resource.

Example:

    #ifdef _WIN32
    auto ok = wui::set_default_theme_from_resource("dark", TXT_DARK_THEME, "JSONS");
    if (!ok)
    {
        std::cerr << "can't load theme" << std::endl;
        return -1;
    }
    #endif

### set_default_theme_from_json
#### Input parameters
 - const std::string &name - theme name
 - const std::string &json - string containing the theme json
#### Return value
 - true if successful

Function loading default application theme from json string

### set_default_theme_from_file
#### Input parameters
 - const std::string &name - theme name
 - const std::string &file_name - путь к json файлу темы
#### Return value
 - true if successful

Function loading default application theme from json file

Example:

    #ifdef _WIN32
    ...
    #else
    auto ok = wui::set_default_theme_from_file("dark", "res/dark.json");
    if (!ok)
    {
        std::cerr << "can't load theme" << std::endl;
        return -1;
    }
    #endif

### set_default_theme_empty
#### Input parameters
 - const std::string &name - theme name

Sets the current theme to an empty theme, e.g. to customize it later using instance setters.

### get_default_theme
#### Return value
 - std::shared_ptr&lt;i_theme&gt; - pointer to the default theme instance

Returns a pointer to the default instance of the topic

### make_custom_theme
#### Input parameters
 - const std::string &name - theme name
#### Return value
 - std::shared_ptr&lt;i_theme&gt; - pointer to the instance of the created theme

Creates and returns a pointer to an empty custom theme. Intended for use in an individual control or a group of controls.

### make_custom_theme
#### Input parameters
 - const std::string &name - theme name
 - const std::string &json - string containing the theme json
#### Return value
 - std::shared_ptr&lt;i_theme&gt; - pointer to the instance of the created theme

Creates and returns a pointer to a custom theme loaded from json. Designed to be used in a single control or a group of controls.

### theme_color
#### Input parameters
 - const std::string &control - control name, e.g. "button"
 - const std::string &value - color name, e.g. "active"
 - std::shared_ptr&lt;i_theme&gt; theme_ = nullptr - pointer to the theme instance, if nullptr - the default theme is taken
#### Return value
 - color - requested color

Returns the color for the ``value`` of the ``control`` control. The ``theme_`` parameter may contain the custom theme instance or be omitted, in which case the color from the default theme will be taken.

Example:

    auto fill_color = enabled_ ? (active || turned_ ? theme_color(tcn, tv_active, theme_) : theme_color(tcn, tv_calm, theme_)) : theme_color(tcn, tv_disabled, theme_);

Where:

    tcn = "button";
    tv_active = "active";
    tv_calm = "calm";
    tv_disabled = "disabled";

A fragment of the theme's json:

    {
      "type": "button",
      "calm": "#06a5df",
      "active": "#1aafe9",
      "disabled": "#a5a5a0"
    }

### theme_dimension
#### Input parameters
 - const std::string &control - control name, e.g. "button"
 - const std::string &value - dimension name, e.g. "border_width"
 - std::shared_ptr&lt;i_theme&gt; theme_ = nullptr - pointer to the theme instance, if nullptr - the default theme is taken
#### Return value
 - int32_t - requested dimension

Returns the size for the ``value`` of the ``control`` control. The ``theme_`` parameter can contain the custom theme instance or be omitted, in which case the size will be taken from the default theme.

### theme_string
#### Input parameters
 - const std::string &control - control name
 - const std::string &value -  string name
 - std::shared_ptr&lt;i_theme&gt; theme_ = nullptr - pointer to the theme instance, if nullptr - the default theme is taken
#### Return value
 - const std::string & - link to the requested string

Returns a string for the ``value`` of the ``control`` control. The ``theme_`` parameter can contain the custom theme instance or be omitted, in which case the string from the default theme will be taken.

### theme_font
#### Input parameters
 - const std::string &control - control name
 - const std::string &value - font name, e.g. "caption_font"
 - std::shared_ptr&lt;i_theme&gt; theme_ = nullptr - pointer to the theme instance, if nullptr - the default theme is taken
#### Return value
 - font - requested font

Returns the font for the ``value`` of the ``control`` control. The ``theme_`` parameter can contain the custom theme instance or be omitted, in which case the font from the default theme will b

### theme_image
#### Input parameters
 - const std::string &name - image name
 - std::shared_ptr&lt;i_theme&gt; theme_ = nullptr - pointer to the theme instance, if nullptr - the default theme is taken
#### Return value
 - const std::vector&lt;uint8_t&gt;& - requested image

Returns a byte array containing the theme image for the ``value`` of the ``control`` control. The theme supports storing images in json as an escaped array. The array can also be loaded into the theme by the instance setter.

Example json:

    "images": [
    { "window_expand": "0x89, 0x50, 0x4E, 0x47, 0x0D, 0x0A, 0x1A, 0x0A, 0x00, 0x00, 0x00, 0x0D, 0x49, 0x48, 0x44, 0x52, 0x00, 0x00, 0x00, 0x19, 0x00, 0x00, 0x00, 0x19, 0x08, 0x06, 0x00, 0x00, 0x00, 0xC4, 0xE9, 0x85, 0x63, 0x00, 0x00, 0x00, 0x09, 0x70, 0x48, 0x59, 0x73, 0x00, 0x00, 0x0B, 0x13, 0x00, 0x00, 0x0B, 0x13, 0x01, 0x00, 0x9A, 0x9C, 0x18, 0x00, 0x00, 0x00, 0x19, 0x74, 0x45, 0x58, 0x74, 0x53, 0x6F, 0x66, 0x74, 0x77, 0x61, 0x72, 0x65, 0x00, 0x41, 0x64, 0x6F, 0x62, 0x65, 0x20, 0x49, 0x6D, 0x61, 0x67, 0x65, 0x52, 0x65, 0x61, 0x64, 0x79, 0x71, 0xC9, 0x65, 0x3C, 0x00, 0x00, 0x00, 0x64, 0x49, 0x44, 0x41, 0x54, 0x78, 0xDA, 0x62, 0xFC, 0xFF, 0xFF, 0x3F, 0x03, 0xAD, 0x01, 0x13, 0x03, 0x1D, 0xC0, 0xA8, 0x25, 0x23, 0xD4, 0x12, 0x16, 0x6C, 0x82, 0x17, 0x2E, 0x9D, 0x9B, 0x00, 0xA4, 0x0C, 0x48, 0x35, 0xCC, 0x40, 0xCF, 0xC8, 0x81, 0x14, 0x9F, 0x18, 0x90, 0x68, 0x89, 0x02, 0x10, 0xDB, 0x93, 0xE4, 0x13, 0x98, 0x87, 0x70, 0xB9, 0x0C, 0x8B, 0xCF, 0x1B, 0x80, 0x54, 0xFD, 0x68, 0xEA, 0x1A, 0xB5, 0x64, 0x90, 0x64, 0x46, 0x58, 0xDA, 0x87, 0x26, 0x4D, 0x62, 0x80, 0x03, 0xB9, 0x96, 0xC8, 0xE3, 0x4B, 0xFB, 0xA4, 0x00, 0xC6, 0xD1, 0x9A, 0x71, 0xD4, 0x92, 0xA1, 0x6D, 0x09, 0x40, 0x80, 0x01, 0x00, 0x94, 0xC7, 0x13, 0xEF, 0xA5, 0x34, 0x2A, 0xAF, 0x00, 0x00, 0x00, 0x00, 0x49, 0x45, 0x4E, 0x44, 0xAE, 0x42, 0x60, 0x82" }
    ]

### The interface of the theme instance

    class i_theme
    {
    public:
        virtual std::string get_name() const = 0;

        virtual void set_color(const std::string &control, const std::string &value, color color_) = 0;
        virtual color get_color(const std::string &control, const std::string &value) const = 0;

        virtual void set_dimension(const std::string &control, const std::string &value, int32_t dimension) = 0;
        virtual int32_t get_dimension(const std::string &control, const std::string &value) const = 0;

        virtual void set_string(const std::string &control, const std::string &value, const std::string &str) = 0;
        virtual const std::string &get_string(const std::string &control, const std::string &value) const = 0;

        virtual void set_font(const std::string &control, const std::string &value, const font &font_) = 0;
        virtual font get_font(const std::string &control, const std::string &value) const = 0;

        virtual void set_image(const std::string &name, const std::vector<uint8_t> &data) = 0;
        virtual const std::vector<uint8_t> &get_image(const std::string &name) = 0;

    #ifdef _WIN32
        virtual void load_resource(int32_t resource_index, const std::string &resource_section) = 0;
    #endif
        virtual void load_json(const std::string &json) = 0;
        virtual void load_file(const std::string &file_name) = 0;
        virtual void load_theme(const i_theme &theme_) = 0;

        virtual bool is_ok() const = 0;

        virtual ~i_theme() {}
    };

This interface accepts a control to receive visual parameters of its display.

### get_name
#### Return value
 - const std::string - theme instance name
Returns the character name of the theme, for example ```"dark"`` or ``"light"``

### set_color
#### Input parameters
 - const std::string &control - control name, e.g. "button"
 - const std::string &value - color name, e.g. "active"
 - color color_ - settable color

Sets the color for the ``value`` of the ``control`` control

Example:

    auto redButtonTheme = wui::make_custom_theme(); // Creating a new custom theme
    redButtonTheme->load_theme(*wui::get_default_theme()); // Load it from the default so that we don't have to set all the values

    redButtonTheme->set_color(wui::button::tc, wui::button::tv_calm, wui::make_color(205, 15, 20)); // Set the fill color of the button in the idle mode

    ... Use a new theme when creating a button or apply it to an existing one

Where:

    wui::button::tc = "button";
    wui::button::tv_calm = "calm";

Which means this color will be applied to the control named ``"button"`` in the ``"calm"`` state.

### get_color
#### Input parameters
 - const std::string &control - control name
 - const std::string &value - color name
#### Return value
 - color - requested color

Returns the color for the ``value`` of the ``control`` control

### set_dimension
#### Input parameters
 - const std::string &control - control name
 - const std::string &value - dimension name
 - int32_t dimension - устанавливаемый размер

Sets the size for the ``value`` of the ``control`` control

### get_dimension
#### Input parameters
 - const std::string &control - control name
 - const std::string &value - dimension name
#### Return value
 - int32_t - requested dimension

Returns the size for the ``value`` of the ``control`` control

### set_string
#### Input parameters
 - const std::string &control - control name
 - const std::string &value - string name
 - const std::string &str - setted string

Sets the string for the ``value`` of the ``control`` control

### get_string
#### Input parameters
 - const std::string &control - control name
 - const std::string &value - string name
#### Return value
 - const std::string & - link to the requested string

Returns a string for the ``value`` of the ``control`` control

### set_font
#### Input parameters
 - const std::string &control - control name
 - const std::string &value - font name
 - const font &font_ - setted font

Sets the font for the ``value`` of the ``control`` control

### get_font
#### Input parameters
 - const std::string &control - control name
 - const std::string &value - font name
#### Return value
 - font - requested font

Returns the font for the ``value`` of the ``control`` control

### set_image
#### Input parameters
 - const std::string &name - image name
 - const std::vector&lt;uint8_t&gt; &data - setted image

Sets the image named ``name`` from the ``data`` byte array

### get_image
#### Input parameters
 - const std::string &name - image name
#### Return value
 - const std::vector&lt;uint8_t&gt; & - requested image

Returns a link to a byte array containing the subject image for the ``value`` of the ``control`` control

### load_resource (Windows only)
#### Input parameters
 - int32_t resource_index - recource ID
 - const std::string &resource_section - resource section, e.g. "JSONS"

Function loading a theme from a json file stored in the application resource

### load_json
#### Input parameters
 - const std::string &json - json string

Function loading theme from json string

### load_file
#### Input parameters
 - const std::string &file_name - json file path

Function loading theme from json file

### load_theme
#### Input parameters
 - const i_theme &theme_ - link to another theme

Function loading a theme from another theme instance

### is_ok
#### Return value
 - true if the json was loaded successfully

Returns whether the theme loaded normally from json
