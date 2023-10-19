file(GLOB SOURCES 
    common/*.cpp
	control/*.cpp
	config/*.cpp
	framework/*.cpp
	graphic/*.cpp
	theme/*.cpp
	theme/impl/*.cpp
	locale/*.cpp
	locale/impl/*.cpp
	system/*.cpp
	window/*.cpp)

find_package(PkgConfig REQUIRED)
pkg_check_modules(CAIRO REQUIRED cairo)

include_directories(.
	../thirdparty
	PUBLIC ${CAIRO_INCLUDE_DIRS})

add_library(wui STATIC ${SOURCES})

