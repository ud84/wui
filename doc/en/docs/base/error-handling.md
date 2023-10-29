##Error handling

WUI does not use exceptions. Methods that may terminate with an error return a bool. To get details about the problem, the get_error() method returns the structure 

    struct error
    {
        error_type type;
        std::string component, message;
        bool is_ok() const;
    };

Errors that may have occurred in the object constructor should be checked like this:

    newObject(new wui::image(IMG_LOGO)).....

    if (!newObject->get_error().is_ok()) { log("error", newObject->get_error().str()); }
