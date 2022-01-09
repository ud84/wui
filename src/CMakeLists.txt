file(GLOB SOURCES 
	common/*.cpp
	control/*.cpp
	graphic/*.cpp
	theme/*.cpp
	theme/impl/*.cpp
	window/*.cpp)

include_directories(../include)

add_library(wui STATIC ${SOURCES})

