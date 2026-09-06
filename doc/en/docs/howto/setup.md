# Installation and build

Use a C++17 compiler and CMake 3.16 or newer. Commands below run from the repository root.
To reproduce the features shown in the current Showcase:

```sh
git clone https://github.com/intent-garden/wui.git
cd wui
git switch I-94
```

## Linux (X11)

The CMake target links Cairo, XCB, X11, threads and udev. For Debian/Ubuntu:

```sh
sudo apt install build-essential cmake pkg-config libcairo2-dev libxcb1-dev   libxcb-cursor-dev libxcb-ewmh-dev libxcb-icccm4-dev libxcb-image0-dev   libx11-dev libx11-xcb-dev libudev-dev
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel
./build/examples/demo/demo
```

An X11 graphical session is required. This is not a native Wayland backend.
Resources are copied beside the example binaries into `res/`.

## Windows

Open `wui.sln` in Visual Studio with C++ desktop tools, a Windows SDK and ATL.
Use the standard Debug/Release configurations with the toolset selected by the
solution (v143 for the regular configurations). Set `demo` as startup project.
The solution also contains legacy v141_xp configurations; those require their own
VS2017/XP toolchain and are not covered by the macOS/browser test runs.
Windows examples embed resources through `.rc` files.

## macOS and browser

- [macOS build, bundles and requirements](macos.md)
- [Emscripten build and static hosting](wasm.md)

## Embed the library in a CMake application

```cmake
set(WUI_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
add_subdirectory(path/to/wui)
add_executable(my_app main.cpp)
target_link_libraries(my_app PRIVATE wui)
```

The `wui` target exports its include path and C++17 requirement. Resource packaging
belongs to the application; see [resources](../base/resources.md). On Windows use
the existing solution settings for Win32 libraries and resource compilation.

`WUI_BUILD_EXAMPLES` defaults to ON. `WUI_BUILD_TESTS` defaults to OFF and enables
macOS or WASM integration tests on those respective toolchains.
