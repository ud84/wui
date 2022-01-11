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

include_directories(../include)
include_directories(.
	PUBLIC ${CAIRO_INCLUDE_DIRS})

add_library(wui STATIC ${SOURCES})

