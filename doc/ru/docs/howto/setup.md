## Получение сборка и проверка

## Зависимости на Linux
Использующие WUI приложения зависят от следующих системных библиотек

    xcb
	xcb-cursor
	xcb-ewmh
	xcb-icccm
	xcb-image
	X11
	X11-xcb
	cairo
	pthread

## Получение и сборка на Linux
    git clone https://github.com/ud84/wui
    cd wui
    cmake CMakeLists.txt
    make

## Проверка на Linux
    cd examples/simple
    ./simple

## Получение и сборка на Windows
    git clone https://github.com/ud84/wui
  
Для сборки и работы рекомендуется Visual Studio не ниже 2017, лучше использовать последнюю версию.
Поддерживается 4 вида сборки:

    Debug
    Debug(v141_xp)
    Release
    Release(v141_xp)

В каждой сборке есть две платформы:

    x64
    x86

Сборки v141_xp позволяют собрать приложение запускающееся на Windows XP. Если вы не планируете
использование этой платформы, лучше использовать сборки Debug / Release.

## Зависимости на Windows

Для работы потребуются следующие компоненты Visual Studio:

    Основные компоненты C++
    MSVC версии 143 - VS2022 C++ x84/x64 Build Tools (последняя версия)
    Пакет SDK для Windows 10 / 11 любой версии
    ATL-библиотека C++ для новейшей версии Build Tools v143 (x86 и x64)

Сборка для XP зависит от:

    Основные компоненты C++
    MSVC версии 141 - средства сборки C++ для VS2017 для x64 или x86
    Пакет SDK для универсальной CRT для Windows
    Поддержка Windows XP на C++ для инструментов VS2017 (версия 141)
    ATL C++ для средств сборки версии 141 (x86 и x64)

## Проверка на Windows
Запустите

    simple.exe
