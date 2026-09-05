# WUI in the browser (WebAssembly)

The experimental browser backend uses Emscripten, Canvas 2D and DOM events. Controls, themes, locales and example application logic remain C++17. Each top-level WUI window is a Canvas region inside the page; embedded windows use the shared controls.

## Build and run

Install and activate the [Emscripten SDK](https://emscripten.org/docs/getting_started/downloads.html). The port is tested with Emscripten 6.0.9.

```sh
emcmake cmake -S . -B build-wasm-release -DCMAKE_BUILD_TYPE=Release
cmake --build build-wasm-release --target wui_web_site -j 6
python3 -m http.server 8080 --bind 127.0.0.1 --directory build-wasm-release/site
```

Open `http://localhost:8080/` for the hello_world, simple and demo examples.

On macOS, Homebrew Emscripten needs a sufficiently recent Python on PATH. If it selects the system Python 3.9, prepend the installed Homebrew Python, for example:

```sh
export PATH="$(brew --prefix python@3.14)/libexec/bin:$PATH"
```

Use the Python version required by your installed Emscripten; replacing system Python is unnecessary.

## Hosting and integration

The `wui_web_site` target produces the self-contained `build-wasm-release/site` directory. Upload its contents to a static host, preserving subdirectories and all `.html`, `.js`, `.wasm` and `.data` files. Serve `.wasm` as `application/wasm`. Use HTTPS for public hosting; browser clipboard access depends on context and user interaction.

An iframe can embed an example into an existing website. For direct embedding, provide `Module.wuiContainer` before loading the script, or a `#wui` element with defined dimensions and `position: relative`. Include the `.wui-*` styles from `src/wasm/shell.html`.

For your own application, use the Emscripten toolchain, link target `wui`, and compile an `int main()` entry point. The library target propagates exception support, Asyncify and the browser JS backend. See `cmake/WuiWasm.cmake` for executable configuration and resource preloading.

`framework::run()` returns after `framework::stop()`. Asyncify preserves stack-owned application objects while yielding to browser events; the existing examples can keep their `MainFrame` on the stack. This compatibility approach adds code size and execution overhead.

Invalidated windows paint through `requestAnimationFrame`. Coordinates are CSS pixels; backing buffers account for `devicePixelRatio`. Browser font metrics can differ from desktop rendering.

Resources are preloaded into virtual `/res`. Browser image decoding is asynchronous and triggers repaint on completion. PNG dimensions are read immediately from the header; other browser-supported formats expose their dimensions after decoding.

## Capabilities and limits

- Shared controls, themes and locales; window movement, resizing, container maximization, minimization with a restore button, embedded/separate modal windows and close vetoes.
- Unicode input, Tab/Shift+Tab, editing and browser copy/cut/paste events. IME uses an auxiliary textarea that displays composition. Automated tests cover composition events; actual system IMEs and mobile keyboards require device testing.
- Synchronous `clipboard_get_text()` returns the last text received through a paste event or written by the application. Use browser Paste or Ctrl/Cmd+V to insert current system clipboard contents. WUI's own context-menu Paste currently uses that saved copy.
- Timers and `emit_event()` use the browser queue. This backend is single-threaded; UI and timers run on the main thread. Pthreads/Worker integration is not implemented. Hidden tabs may throttle timers.
- Files and INI settings are in memory and reset on reload. IndexedDB persistence and user file import/export are follow-up work.
- Desktop tray/taskbar integration, native notifications and device hotplug are unavailable. Tray/taskbar calls create no OS elements; enabling device notifications reports an error.
- A complete accessibility tree for Canvas controls is not yet implemented. Browser support remains experimental.

## Tests

```sh
emcmake cmake -S . -B build-wasm -DCMAKE_BUILD_TYPE=Debug -DWUI_BUILD_TESTS=ON
cmake --build build-wasm -j 6
npm --prefix tests/wasm ci
npx --prefix tests/wasm playwright install chromium firefox webkit
node tests/wasm/smoke.mjs build-wasm
WUI_BROWSER=firefox node tests/wasm/smoke.mjs build-wasm
WUI_BROWSER=webkit node tests/wasm/smoke.mjs build-wasm
```

The test starts and stops a local HTTP server. It checks all three examples, browser errors, rendering and screenshots. With `WUI_BUILD_TESTS=ON`, it also checks clicks, Unicode, clipboard, composition events, focus, modal routing, resize, timers and `run()` completion. Playwright WebKit tests the engine, not the installed Safari application.
