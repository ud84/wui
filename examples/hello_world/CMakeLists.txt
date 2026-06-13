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
	udev
)

add_custom_target(copy_hello_world ALL
   ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/res ${CMAKE_CURRENT_BINARY_DIR}/res
   DEPENDS hello_world
   COMMENT "Copy ${CMAKE_CURRENT_SOURCE_DIR}/res to ${CMAKE_CURRENT_BINARY_DIR}/res"
   )
