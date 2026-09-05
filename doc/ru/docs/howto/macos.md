# macOS

WUI использует AppKit для окон, событий и ввода, Core Graphics для буферов отрисовки
и ImageIO для изображений. X11, Cairo и сторонние GUI-фреймворки не нужны.
Общие контролы и передача `rect` по значению сохранены.

## Сборка

Нужны Xcode Command Line Tools и CMake 3.16 или новее. Минимальная версия macOS,
выставляемая сборкой, — 10.15. Проверка выполнения проводилась на Apple Silicon;
для Intel можно собрать отдельную или универсальную библиотеку.

```sh
cmake -S . -B build-macos -DCMAKE_BUILD_TYPE=Release
cmake --build build-macos --parallel
open build-macos/examples/demo/demo.app
```

Также собираются `examples/hello_world/hello_world.app` и
`examples/simple/simple.app`. Ресурсы копируются в `Contents/Resources/res`.
Относительные пути ресурсов сначала ищутся в рабочем каталоге, затем в пакете
приложения. Текущий каталог процесса библиотека не меняет.

Для универсальной сборки:

```sh
cmake -S . -B build-macos-universal -DCMAKE_OSX_ARCHITECTURES="arm64;x86_64" \
  -DWUI_BUILD_EXAMPLES=OFF -DCMAKE_BUILD_TYPE=Release
cmake --build build-macos-universal --parallel
```

Для подключения из другого CMake-проекта:

```cmake
set(WUI_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
add_subdirectory(path/to/wui)
target_link_libraries(my_app PRIVATE wui)
```

C++-заголовки не требуют Objective-C++. Файлы `.mm` и фреймворки macOS подключаются
внутри цели `wui`.

## Поведение

- Вызывайте `framework::init()`, `framework::run()` и методы UI в главном потоке.
  Коллбэки контролов и `wui::timer` также выполняются в нём. Из рабочего потока можно
  вызвать `window::emit_event()`; сообщение будет доставлено асинхронно в главный поток.
- Координаты задаются в логических точках. Буферы учитывают Retina-масштаб экрана;
  при смене масштаба пересоздаются. `font::size` соответствует высоте строки WUI.
- Поддерживаются UTF-8, ввод через `NSTextInputClient`, композиция IME,
  Cmd+A/C/X/V, Tab/Shift+Tab, мышь и вертикальная прокрутка.
- Поддерживаются встроенные окна, физические модальные окна, изменение размера,
  перемещение за заголовок, сворачивание и разворачивание. Декорации рисует WUI.
- Измерение текста не зависит от открытого окна. Можно рассчитать размеры диалога
  до его создания и после закрытия других окон.
- Цвета используют существующую POSIX-семантику WUI: `make_color(r,g,b)` непрозрачен,
  четвёртый аргумент задаёт прозрачность (`0` — непрозрачно, `255` — прозрачно).
- `tray_icon` создаёт иконку в строке меню. `show_message()` показывает popover у
  иконки, а не уведомление Notification Center.
- INI-конфигурация использует переданный путь. Для запуска через Finder задавайте
  абсолютный доступный для записи путь (например, внутри Application Support),
  а не рассчитывайте на рабочий каталог.

## Проверка

```sh
cmake -S . -B build-macos -DWUI_BUILD_TESTS=ON -DCMAKE_BUILD_TYPE=Debug
cmake --build build-macos --parallel
ctest --test-dir build-macos --output-on-failure
```

Тесты требуют активной графической сессии macOS. Они открывают временные окна,
проверяют события, UTF-8, буфер обмена, диалоги, таймеры, пиксели offscreen-буферов
и запускают все три примера. Снимки сохраняются в `/tmp/wui-macos-*.png`.

## Ограничения

Уведомления о подключении устройств пока не реализованы:
`enable_device_change_handling(true)` сообщает об этом через `window::get_error()`.
Dock — свойство приложения, поэтому `hide_taskbar_icon()`/`show_taskbar_icon()`
не меняют его для отдельного окна. Полноценный accessibility-интерфейс контролов
и горизонтальная прокрутка трекпадом пока не реализованы.
