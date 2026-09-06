# Установка и сборка

Нужны компилятор C++17 и CMake 3.16 или новее. Команды выполняются из корня репозитория.
Чтобы получить функции текущего Showcase:

```sh
git clone https://github.com/intent-garden/wui.git
cd wui
git switch I-94
```

## Linux (X11)

CMake подключает Cairo, XCB, X11, потоки и udev. Для Debian/Ubuntu:

```sh
sudo apt install build-essential cmake pkg-config libcairo2-dev libxcb1-dev   libxcb-cursor-dev libxcb-ewmh-dev libxcb-icccm4-dev libxcb-image0-dev   libx11-dev libx11-xcb-dev libudev-dev
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel
./build/examples/demo/demo
```

Нужна графическая сессия X11. Нативного бэкенда Wayland пока нет.
Ресурсы примеров копируются в `res/` рядом с исполняемыми файлами.

## Windows

Откройте `wui.sln` в Visual Studio с инструментами C++ для рабочего стола,
Windows SDK и ATL. Используйте обычные Debug/Release и toolset из solution
(v143 для стандартных конфигураций). Назначьте `demo` стартовым проектом.
Есть также старые конфигурации v141_xp: им нужны отдельные инструменты VS2017/XP;
проверки macOS и браузера не проверяют работоспособность этих сборок.
Ресурсы Windows-примеров встроены через `.rc`.

## macOS и браузер

- [Сборка macOS, bundles и требования](macos.md)
- [Сборка Emscripten и статический хостинг](wasm.md)

## Подключение к CMake-приложению

```cmake
set(WUI_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
add_subdirectory(path/to/wui)
add_executable(my_app main.cpp)
target_link_libraries(my_app PRIVATE wui)
```

Цель `wui` передаёт путь заголовков и требование C++17. Упаковкой ресурсов занимается
приложение: см. [ресурсы](../base/resources.md). На Windows используйте настройки
существующего solution для системных библиотек и компиляции ресурсов.

`WUI_BUILD_EXAMPLES` по умолчанию ON, `WUI_BUILD_TESTS` — OFF. Тесты доступны для
macOS и WASM при выборе соответствующей платформы сборки.
