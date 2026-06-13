file(GLOB SOURCES 
	*.cpp)

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
	udev
)

add_custom_target(copy_simple ALL
   ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/res ${CMAKE_CURRENT_BINARY_DIR}/res
   DEPENDS simple
   COMMENT "Copy ${CMAKE_CURRENT_SOURCE_DIR}/res to ${CMAKE_CURRENT_BINARY_DIR}/res"
   )
