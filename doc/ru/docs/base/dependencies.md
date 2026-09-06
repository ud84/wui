# Зависимости

WUI — компилируемая библиотека. В `thirdparty` находятся nlohmann/json,
UTF-8-инструменты и Boost.Nowide для преобразования строк Windows.
Полный дистрибутив Boost для использования WUI не нужен.

| Бэкенд | Системные зависимости |
| --- | --- |
| Windows | Win32, GDI+, Windows SDK, toolchain и ATL из настроек solution |
| Linux | Cairo, XCB (cursor/EWMH/ICCCM/image), X11/X11-xcb, потоки, udev; pkg-config для CMake |
| macOS | Cocoa/AppKit, CoreGraphics, CoreText, ImageIO; без X11 и Cairo |
| WASM | Emscripten для сборки; Canvas 2D, DOM и WebAssembly во время работы |

Команды приведены в [установке](../howto/setup.md) и руководствах платформ.
