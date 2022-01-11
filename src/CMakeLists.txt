file(GLOB SOURCES 
	common/*.cpp
	control/*.cpp
	graphic/*.cpp
	theme/*.cpp
	theme/impl/*.cpp
	system/*.cpp
	window/*.cpp
	../thirdparty/xcbft/src/utf8_utils/*.c
	../thirdparty/xcbft/src/xcbft/*.c)

include_directories(../include)

add_library(wui STATIC ${SOURCES})

