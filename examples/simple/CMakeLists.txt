file(GLOB SOURCES 
	*.cpp)

include_directories(.)

add_executable(simple ${SOURCES})

target_link_libraries(simple
	wui
	xcb
	xcb-cursor
	xcb-ewmh
	X11
	X11-xcb
	pangocairo-1.0
	pango-1.0
	gobject-2.0
	glib-2.0
	harfbuzz
	cairo
	pthread
)
