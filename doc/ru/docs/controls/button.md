# Кнопка

## Интерфейс

    class button : public i_control, public std::enable_shared_from_this<button>
    {
    public:
        button(const std::string &caption, std::function<void(void)> click_callback, const std::string &theme_control_name = tc, std::shared_ptr<i_theme> theme_ = nullptr);
        button(const std::string &caption, std::function<void(void)> click_callback, button_view button_view_, const std::string &theme_control_name = tc, std::shared_ptr<i_theme> theme_ = nullptr);

    #ifdef _WIN32
        button(const std::string &caption, std::function<void(void)> click_callback, button_view button_view_, int32_t image_resource_index, int32_t image_size, const std::string &theme_control_name = tc, std::shared_ptr<i_theme> theme_ = nullptr);
    #endif
        button(const std::string &caption, std::function<void(void)> click_callback, button_view button_view_, const std::string &  image_file_name, int32_t image_size, const std::string &theme_control_name = tc, std::shared_ptr<i_theme> theme_ = nullptr);
        button(const std::string &caption, std::function<void(void)> click_callback, button_view button_view_, const std::vector<uint8_t> &image_data, int32_t image_size, const std::string &theme_control_name = tc, std::shared_ptr<i_theme> theme_ = nullptr);
        ~button();

        /// Button's interface
        void set_caption(const std::string &caption);

        void set_button_view(button_view button_view_);
    #ifdef _WIN32
        void set_image(int32_t resource_index);
    #endif
        void set_image(const std::string &file_name);
        void set_image(const std::vector<uint8_t> &image_data);

        void enable_focusing();
        void disable_focusing();

        void turn(bool on);
        bool turned() const;

        void set_callback(std::function<void(void)> click_callback);
    };

Кнопка может быть следующих видов:
    
    text
    image
    image_right_text
    image_bottom_text
    switcher
    radio
    anchor
    sheet

<img src="../../img/button.png">
