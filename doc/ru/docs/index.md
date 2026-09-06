# Документация WUI

WUI — компилируемая C++17-библиотека GUI с общими контролами для Windows, Linux,
macOS и WebAssembly. Лицензия — Boost Software License 1.0; WUI не входит в Boost
и не является header-only библиотекой.

[Открыть живой Showcase](https://libwui.org/wasm/demo/) · [Исходный код](https://github.com/intent-garden/wui)

## Начало работы

- [Установка и сборка](howto/setup.md)
- [Hello world](howto/hello-world.md)
- [Бэкенд macOS](howto/macos.md)
- [WebAssembly и ограничения браузера](howto/wasm.md)
- [Устройство приложения](article/onboarding.md)
- [Все контролы](controls/all.md)

## Возможности WUI

В Showcase есть девять видов кнопок, включая чекбокс, тоггл и radio, нарисованные
примитивами, Unicode-ввод, фильтрация списков, меню, диалоги, собственная графика
и разделители областей. Семь вкладок можно попробовать прямо в браузере.

Windows и Linux — исходные бэкенды. Порты macOS и браузера экспериментальные;
интеграционные проверки выполняются на macOS и в Chromium, Firefox и WebKit.
Требования и ограничения описаны на страницах платформ.
