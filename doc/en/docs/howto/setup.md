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
  
To build and work, Visual Studio is recommended at least 2017, it is better to use the latest version.
4 types of build types are supported:

    Debug
    Debug(v141_xp)
    Release
    Release(v141_xp)

There are two platforms in each build type:

    x64
    x86

The v141_xp builds allow you to build an application that runs on Windows XP.
If you do not plan to use this platform, it is better to use Debug / Release builds.

## Dependencies on Windows

The following Visual Studio components will be required:

    Basic C++ components
    MSVC version 143 - VS2022 C++ x84/x64 Build Tools (latest version)
    SDK for Windows 10 / 11 of any version
    ATL C++ library for the latest version of Build Tools v143 (x86 and x64)

The build for XP depends on:

    Basic C++ components
    MSVC version 141 - C++ build tools for VS2017 for x64 or x86
    SDK for Universal CRT for Windows
    C++ support for Windows XP for VS2017 tools (version 141)
    ATL C++ for build tools version 141 (x86 and x64)

## Test on Windows
Run

    simple.exe
