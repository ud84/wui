# Hello world

Возьмите `examples/hello_world` как основу приложения с темами, локалями,
настройками и упаковкой ресурсов. Соберите цель `hello_world` по
[инструкции установки](setup.md) или [откройте пример в браузере](https://libwui.org/wasm/hello_world/).

## Минимальный пример C++

Пример ниже использует полный `res/dark.json` из одного из готовых примеров.
Сохраните связанные ресурсы. Этот `main()` подходит для macOS, Linux и WASM;
для GUI-подсистемы Windows используйте WinMain из комплектного примера.

```cpp
#include <wui/framework/framework.hpp>
#include <wui/window/window.hpp>
#include <wui/control/text.hpp>
#include <wui/control/button.hpp>
#include <wui/theme/theme.hpp>
#include <memory>

int main()
{
    wui::framework::init();
    if (!wui::set_default_theme_from_file("dark", "res/dark.json")) return 1;
    auto window = std::make_shared<wui::window>();
    auto label = std::make_shared<wui::text>("Hello, WUI!",
        wui::hori_alignment::center, wui::vert_alignment::center);
    auto button = std::make_shared<wui::button>("Say hello",
        [label] { label->set_text("Hello from shared C++ controls!"); });
    window->add_control(label, {20, 50, 400, 110});
    window->add_control(button, {110, 130, 310, 170});
    if (!window->init("Hello WUI", {-1, -1, 420, 220}, wui::window_style::frame,
        [] { wui::framework::stop(); })) return 1;
    wui::framework::run();
}
```

Контролы принимают `{left, top, right, bottom}`. Вызов `window::init()` допускает
специальный вариант `{-1, -1, width, height}` для центрирования окна.

Окна, диалоги и данные callback должны жить до завершения `framework::run()`.
В callback закрытия вызывается `framework::stop()`. Создание окна не заменяет
цикл событий. WASM сохраняет такой порядок жизни объектов благодаря Asyncify.

Полный пример добавляет переключение темы и языка, INI/реестр, Unicode-ввод имени
и диалоги подтверждения. Исходники: `hw.cpp`, `MainFrame/MainFrame.h`,
`MainFrame/MainFrame.cpp`, `Resource.h`.

На macOS ресурсы находятся в `Contents/Resources/res`, на Linux — рядом с бинарником,
на Windows — в `.rc`, в WASM — в виртуальном `/res`. См. [ресурсы](../base/resources.md).
