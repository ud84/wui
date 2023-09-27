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
  
Откройте wui.sln при помощи Visual Studio и соберите проект

## Проверка на Windows
Запустите

    simple.exe
