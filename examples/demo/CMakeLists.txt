file(GLOB SOURCES 
	*.cpp
	ButtonSheet/*.cpp
	InputSheet/*.cpp
	MainFrame/*.cpp
	MainSheet/*.cpp)

include_directories(.)

add_executable(demo ${SOURCES})

target_link_libraries(demo
	wui
	xcb
	xcb-cursor
	xcb-ewmh
	xcb-icccm
	xcb-image
	X11
	X11-xcb
	stdc++fs
	cairo
	pthread
	udev
)
