# WUI
Window User Interface Library

WUI is a cross-platform modern C++ library for creating graphical user interfaces.

WUI is an attempt to make as easy to use cross-platform library as possible for creating a modern graphical user interface in C++. The library uses modern C++ (C++11) and has a minimalistic API. The main design principle is to make it as simple as possible, but not simpler. The library does not try to build a complete abstraction over the operating system and deals only with basic UI stuff. It has nothing about network, strings, file handling, multithreading, and other useful but irrelevant UI things, which are also available in the modern C++ standard.

WUI is based on two primitives - Window and Control.

A window can contain controls, also window itself is a control. Control is any visual element for interaction with the user - a button, an input field, a list, etc. Window - provides the physical work of drawing on the system graphical context, accepting input events, and controlling input focus. Control knows how to process events coming from Window, stores its states and draws itself on a graphical context which is provided by the containing Window.

The library also has auxiliary tools for work - structures Common (containing such basic types as Rect, Color), Event (Mouse, Keyboard, Internal and system events), Graphic (for physical rendering on the system graphical context) and Theme (a system of constants for convenient support of visual themes).

To view more information go to the wiki: https://github.com/ud84/WUI/wiki

![alt text]([http://url/to/img.png](https://libwui.org/scrscht/2.png)https://libwui.org/scrscht/2.png)
![alt text]([http://url/to/img.png](https://libwui.org/scrscht/2.png)https://libwui.org/scrscht/1.png)
