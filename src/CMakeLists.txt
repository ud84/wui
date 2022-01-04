file(GLOB SOURCES 
	control/*.cpp
	theme/*.cpp
	theme/impl/*.cpp
	window/*.cpp)

include_directories(../include)

add_library(wui STATIC ${SOURCES})

