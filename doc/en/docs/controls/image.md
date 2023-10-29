# Image

image is needed to display icons in a uniform way, taking into account the visual theme. For example button uses image to draw icons on itself. image draws itself from a resource that matches the visual theme. 

An example of using image:
Create in the constructor of the class containing image

    logoImage(new wui::image(IMG_LOGO))...

IMG_LOGO is defined in the application's resourse.h as follows:

    #ifdef _WIN32
    #define IMG_LOGO				  109
    #else // _WIN32
    static constexpr const char* IMG_LOGO = "logo.png";
    #endif

Thus, the image will be taken from an exe resource on Windows or from a file on other systems.
The magic of changing the image when changing the theme is realized as follows. image has its own settings in theme:

## light.json:

    {
        "type": "image",
        "resource": "IMAGES_LIGHT",
        "path": "res/images/light"
    }

## dark.json:

    {
        "type": "image",
        "resource": "IMAGES_DARK",
        "path": "res/images/dark"
    }

The path to the resource file is made up of the path specified in theme and the file name in image, which leads to automatic replacement of all application images when the theme is changed.
On Windows it is worth mentioning how the rc file of the application is organized.

    IMG_LOGO IMAGES_DARK   "res\\images\\dark\\logo.png"
    IMG_LOGO IMAGES_LIGHT  "res\\images\\light\\logo.png"

Thus, replacing the IMAGES_DARK / IMAGES_LIGHT group causes a similar effect as with files, without having to change the resource ID.

