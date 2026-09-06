# Dependencies

WUI itself is a compiled library. Bundled dependencies under `thirdparty` include
nlohmann/json, UTF-8 helpers and Boost.Nowide support used for Windows string
conversion. Using WUI does not require the complete Boost distribution.

| Backend | System dependencies |
| --- | --- |
| Windows | Win32, GDI+, the Windows SDK and solution toolchain/ATL settings |
| Linux | Cairo, XCB (cursor/EWMH/ICCCM/image), X11/X11-xcb, threads, udev; pkg-config for CMake |
| macOS | Cocoa/AppKit, CoreGraphics, CoreText, ImageIO; no X11 or Cairo |
| WASM | Emscripten at build time; Canvas 2D, DOM and WebAssembly at runtime |

See [installation](../howto/setup.md) for commands and platform-specific guides.
