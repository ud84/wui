# ![alt text](https://libwui.org/img/wui_logo_black.png) WUI
Window User Interface Library

![#1589F0](https://placehold.co/15x15/1589F0/1589F0.png) WUI is a cross-platform modern C++ library for creating graphical user interfaces.
Now it supoort only Windows and Linux*.

![#1589F0](https://placehold.co/15x15/1589F0/1589F0.png) WUI is an attempt to make as easy to use cross-platform library as possible for creating a modern graphical user interface in C++. The library uses modern C++ (C++11) and has a minimalistic API. The main design principle is to make it as simple as possible, but not simpler. The library does not try to build a complete abstraction over the operating system and deals only with basic UI stuff. It has nothing about network, strings, file handling, multithreading, and other useful but irrelevant UI things, which are also available in the modern C++ standard.

![#1589F0](https://placehold.co/15x15/1589F0/1589F0.png) WUI is based on two primitives - Window and Control.

![#1589F0](https://placehold.co/15x15/1589F0/1589F0.png) A window can contain controls, also window itself is a control. Control is any visual element for interaction with the user - a button, an input field, a list, etc. Window - provides the physical work of drawing on the system graphical context, accepting input events, and controlling input focus. Control knows how to process events coming from Window, stores its states and draws itself on a graphical context which is provided by the containing Window.

![#1589F0](https://placehold.co/15x15/1589F0/1589F0.png) The library also has auxiliary tools for work - structures Common (containing such basic types as Rect, Color), Event (Mouse, Keyboard, Internal and system events), Graphic (for physical rendering on the system graphical context) and Theme (a system of constants for convenient support of visual themes).

![#1589F0](https://placehold.co/15x15/1589F0/1589F0.png) To view more information go to the wiki: https://github.com/ud84/WUI/wiki

![#1589F0](https://placehold.co/15x15/1589F0/1589F0.png) Web site: https://libwui.org

![#c5f015](https://placehold.co/15x15/c5f015/c5f015.png) In order for me to implement macOS support I need a macbook, which I don't have.
If you want to donate a macbook to me on this wallet: TAiA5MuS3nCkAaPWSjfLmGijvTzMychLbY TRC20

Example screenshot (Video call application)
![alt text](https://libwui.org/img/screenshoot-1.png)
![alt text](https://libwui.org/img/screenshoot-2.png)
