# ![alt text](https://libwui.org/main/img/wui_logo_black_small.png) WUI
Window User Interface Library

![#1589F0](https://placehold.co/15x15/1589F0/1589F0.png) WUI is an attempt to make as easy to use cross-platform library as possible for creating a modern graphical user interface in C++. The library uses  C++11 and has a minimalistic API. Now it support only Windows and Linux*.

![#1589F0](https://placehold.co/15x15/1589F0/1589F0.png) Everything is based on two entities - Window and Control. A window can contain controls, and the window itself is a control.

![#1589F0](https://placehold.co/15x15/1589F0/1589F0.png) Control is any visual element for user interaction - button, input field, list, menu, etc. Control knows how to handle events coming from Window, stores its states, and draws itself on the graphical context provided by the window containing it.

![#1589F0](https://placehold.co/15x15/1589F0/1589F0.png) Window - receives system events and provides their distribution to subscribers. The window also commands its controllers to redraw and provides them with their own graphic. In addition, the window controls the input focus, can do modality and send an event to the subscribed user or to the system.

![#1589F0](https://placehold.co/15x15/1589F0/1589F0.png) Graphic is the third base entity that provides an interface to the system's drawing methods. Currently, drawing is implemented on Windows GDI/GDI+ and Linux xcb/cairo. Of course, there is no obstacle to implement drawing on vulcan/bare metal/etc.
![#1589F0](https://placehold.co/15x15/1589F0/1589F0.png) The library also has auxiliary tools for work - structures common (containing such basic types as rect, color), event (mouse, keyboard, internal and system events), graphic (for physical rendering on the system graphical context) and Theme (a system of constants for convenient support of visual themes).

![alt text](https://libwui.org/doc/img/system.png)
I didn't do a full-blown UML for the sake of brevity.

![#1589F0](https://placehold.co/15x15/1589F0/1589F0.png) To view more information go to the wiki: https://libwui.org/doc

![#1589F0](https://placehold.co/15x15/1589F0/1589F0.png) Web site: https://libwui.org

![#c5f015](https://placehold.co/15x15/c5f015/c5f015.png) In order for me to implement macOS support I need a macbook, which I don't have.
If you want to donate a macbook to me on this place: https://libwui.org/donate

Example screenshot (Video call application)
![alt text](https://libwui.org/main/img/screenshoot-1.png)
![alt text](https://libwui.org/main/img/screenshoot-2.png)
