file(GLOB SOURCES 
	*.cpp)

#include_directories(.)
include_directories(.)

add_executable(simple ${SOURCES})

target_link_libraries(simple
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
)
