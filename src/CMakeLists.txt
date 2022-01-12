file(GLOB SOURCES 
	common/*.cpp
	control/*.cpp
	graphic/*.cpp
	theme/*.cpp
	theme/impl/*.cpp
	system/*.cpp
	window/*.cpp)

find_package(PkgConfig REQUIRED)
pkg_check_modules(CAIRO REQUIRED cairo)

include_directories(.
#	PUBLIC ${CAIRO_INCLUDE_DIRS})
	/usr/include/cairo
	/usr/include/glib-2.0
	/usr/lib/x86_64-linux-gnu/glib-2.0/include
	/usr/include/pixman-1
	/usr/include/uuid
	/usr/include/freetype2
	/usr/include/libpng1)

add_library(wui STATIC ${SOURCES})

