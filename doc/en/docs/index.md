# Welcome to WUI

WUI is an attempt to make an easy to use and as fast as possible cross-platform library for creating a modern C++ graphical user interface. The library uses modern C++ (C++11) and has a minimalistic API. The library doesn't try to build a complete abstraction over the operating system and only deals with basic UI stuff. It doesn't have anything about networking, strings, file handling, which are in modern C++ standard and boost.

WUI is based on two primitives - Window and Control.

A window can contain controls, also a window itself is a control. A control is any visual element for user interaction - a button, an input field, a list, etc. Window - provides the physical work of drawing on the graphical context of the system, accepting input events, and controlling the input focus. Control knows how to handle events coming from Window, stores its states, and draws itself on the graphical context provided by the containing window.

The library also has auxiliary tools for operation - structures Common (contains such basic types as Rect, Color), Event (mouse, keyboard, internal and system events), Graphic (for physical drawing on the system graphic context) Theme (a system of constants for convenient support of visual themes) and Locale (a subsystem for convenient storage of textual content).

## Platforms

The following platforms are currently supported:

* Windows (WinAPI + GDI)
* Linux (X11 + xcb)

Work on MacOS platform will be completed soon.

All platform-dependent code is collected in two elements - window and graphic (rendering subsystem).

## Quick start

* [Receive, build and setup](howto/setup.md)

## Contacts:

* Email: [info@libwui.org](mailto:info@libwui.org)
* GitHub tracker: [https://github.com/ud84/wui/issues](https://github.com/ud84/wui/issues)
* Telegram: [Official WUI channel](https://t.me/libwui)
