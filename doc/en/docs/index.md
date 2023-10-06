# Welcome to WUI

WUI is an attempt to make an easy to use and as fast as possible cross-platform library for creating a modern C++ graphical user interface. The library uses C++11 and has a minimalistic API.

WUI is based on two primitives - Window and Control.

A window can contain controls, also a window itself is a control. A [control](base/interfaces.md#control) is any visual element for user interaction - a button, an input field, a list, etc. [window](base/interfaces.md#window) - provides the physical work of drawing on the graphical context of the system, accepting input events, and controlling the input focus. Control knows how to handle events coming from Window, stores its states, and draws itself on the graphical context provided by the containing window.

The library also has auxiliary tools for operation - structures [common](base/common.md) (contains such basic types as ``rect``, ``color``, ``font``), [event](base/event.md) (``mouse``, ``keyboard``, ``internal`` and ``system events``), [graphic](base/graphic.md) (for physical drawing on the system graphic context) [theme](base/theme.md) (a system of constants for convenient support of visual themes) and ``locale`` (a subsystem for convenient storage of textual content).

<img src="img/system.png">

## Platforms

The following platforms are currently supported:

* Windows (WinAPI + GDI)
* Linux (X11 + xcb)

Work on MacOS platform will be completed soon.

All platform-dependent code is collected in two elements - [window](base/interfaces.md) and [graphic](base/graphic.md) (rendering subsystem).

## Quick start

* [Receive, build and setup](howto/setup.md)

## Contacts:

* Email: [info@libwui.org](mailto:info@libwui.org)
* GitHub tracker: [https://github.com/ud84/wui/issues](https://github.com/ud84/wui/issues)
* Telegram: [Official WUI channel](https://t.me/libwui)
