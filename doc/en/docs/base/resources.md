# Resources

The theme and locale subsystems are implemented for convenient and uniform display of many controls and inscriptions of the application, for convenience of work of non-programmers, e.g. designers, translators. 

The application always has a current theme and locale. These are essentially configs that give values for a pair of section + key. A theme is passed to each control (and window, since it is also a control) to give that control values for its colors, sizes, thicknesses, fonts, and so on. By default, you may not pass a custom theme to a control, then it will use the common current theme of the application.

The custom code to generate the labels has access to the current application locale. This allows all text resources to be collected in one place and also, in one place, changed for the entire application.

The application has the ability to change the current theme and locale, which causes an automatic change in the appearance of controls and windows / language of the entire application.

Technically the subsystems are implemented similarly, let's take theme as an example
The theme is a json containing parameter values for controls, for example for window and inscription and images.

## dark theme:
    {
        "controls": [
        {
            "type": "window",
            "background": "#131519",
            "border": "#404040",
            "border_width": 1,
            "text": "#f5f5f0",
            "active_button": "#3b3d41",
            "caption_font": {
                "name": "Segoe UI",
                "size": 18,
                "decorations": "normal"
            }
        },
        {
            "type": "text",
            "color": "#f5f5f0",
            "font": {
                "name": "Segoe UI",
                "size": 18
            }
        },
        {
            "type": "image",
            "resource": "IMAGES_DARK",
            "path": "~/.hello_wui/res/images/dark"
        }
        /* И так далее */
    }

## light theme:

    {
        "controls": [
        {
            "type": "window",
            "background": "#fffffe",
            "border": "#9a9a9a",
            "border_width": 1,
            "text": "#191914",
            "caption_font": {
                "name": "Segoe UI",
                "size": 18
            }
        },
        {
            "type": "text",
            "color": "#191914",
            "font": {
                "name": "Segoe UI",
                "size": 18
            }
        },
        {
            "type": "image",
            "resource": "IMAGES_LIGHT",
            "path": "~/.hello_wui/res/images/light"
        }
        /* И так далее */
    }

This approach provides the application and controls with a transparent, centralized mechanism for controlling the display. If you need to create a custom control (e.g. a red button), you can simply add a new section to the json:

    {
        "type": "red_button",
        "calm": "#c61818",
        "active": "#e31010",
        "border": "#c90000",
        "border_width": 1,
        "focused_border": "#dcd2dc",
        "text": "#f0f1f1",
        "disabled": "#a5a5a0",
        "round": 0,
        "focusing": 1,
        "font": {
            "name": "Segoe UI",
            "size": 18
        }
    }

And when creating a control, specify the name of the control: "red_button", for example:

    cancelButton(new wui::button(wui::locale("button", "cancel"), [this]() { window->destroy(); }, "red_button"))

The image controller is used to work with icons and similar images. It also uses theme to get the win32 resource identifier or path to the image file. This allows you to create an image

    logoImage(new wui::image(IMG_LOGO)) 

Where:

    #ifdef _WIN32
    #define IMG_LOGO 4010
    #else
    static constexpr const char* IMG_LOGO = "logo.png";
    #endif

The logo will be uploaded according to the given theme.
