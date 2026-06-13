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

add_custom_target(copy_demo ALL
   ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/res ${CMAKE_CURRENT_BINARY_DIR}/res
   DEPENDS demo
   COMMENT "Copy ${CMAKE_CURRENT_SOURCE_DIR}/res to ${CMAKE_CURRENT_BINARY_DIR}/res"
   )
