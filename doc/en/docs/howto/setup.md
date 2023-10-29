## Receive build and verify

## Dependencies on Linux
WUI applications that use the WUI depend on the following system libraries

    xcb
	xcb-cursor
	xcb-ewmh
	xcb-icccm
	xcb-image
	X11
	X11-xcb
	cairo
	pthread

## Receive and build on Linux
    git clone https://github.com/ud84/wui
    cd wui
    cmake CMakeLists.txt
    make

## Checking on Linux
    cd examples/simple
    ./simple

## Receive and build on Windows
    git clone https://github.com/ud84/wui
  
Open wui.sln using Visual Studio and build the project

## Test on Windows
Run

    simple.exe
