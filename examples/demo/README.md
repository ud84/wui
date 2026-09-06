# WUI Showcase

An interactive gallery built entirely with WUI. The same C++ source runs on
Windows, Linux, macOS and WebAssembly; the browser host only supplies a canvas.

| Page | Controls and interactions |
| --- | --- |
| Overview | Custom panel drawing, sine/square wave selection, slider and linked progress |
| Windows | Four message dialogs with result callbacks and a separate editable scratchpad |
| Buttons | All eight button views, enabled/disabled state, counters, switches and radio groups |
| Inputs | Single/multiline, password/reveal, read-only, integer, numeric, hexadecimal and host/port filters |
| Lists | 120 sample rows, text/category filtering, selection, activation and scrolling |
| Menus | Nested items, separators, disabled actions, tooltips and theme-aware images |
| Layout | Resizable split view, centered slider, signed progress and a standalone scrollbar |

The title-bar theme button switches between light and dark. Page changes retain
text, selected values and the split position. Non-active controls are detached
from the window, including their internal menus and scrollbars.

The gallery captions are English. Input samples include several writing systems;
standard control menus/dialog buttons use the configured WUI locale. The tray
icon is not instantiated: this example is also embedded in a browser, which has
no system tray. Project rows and menu commands are demonstration data, not file
or network operations.

Build with the normal project CMake configuration, then build target `demo`.
For the browser build, serve `build-wasm-release` over HTTP and open
`examples/demo/demo.html` (see the repository's WASM build documentation).

`tests/wasm/smoke.mjs` exercises every page in Chromium, Firefox or WebKit via
`WUI_BROWSER`. It checks editing, filters (including empty results), menu actions,
dialog results, child window lifecycle, theme changes and state preservation.
