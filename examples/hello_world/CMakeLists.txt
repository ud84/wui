file(GLOB SOURCES 
	*.cpp
	MainFrame/*.cpp)

include_directories(.)

add_executable(hello_world ${SOURCES})

target_link_libraries(hello_world
	wui
	xcb
	xcb-cursor
	xcb-ewmh
	xcb-icccm
	xcb-image
	X11
	X11-xcb
	cairo
	pthread
	stdc++fs
)
