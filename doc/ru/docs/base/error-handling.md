##Обработка ошибок

WUI не использует исключения. Методы, которые могут завершиться ошибкой возвращают bool. Для получения подробностей о возникшей проблеме используется метод get_error() возвращающий структуру 

    struct error
    {
        error_type type;
        std::string component, message;
        bool is_ok() const;
    };

Ошибки, возможно возникшие в конструкторе объекта, нужно проверять так:

    newObject(new wui::image(IMG_LOGO))...

    if (!newObject->get_error().is_ok()) { log(“error”, newObject->get_error().str()); }
