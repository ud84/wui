file(GLOB SOURCES 
	AudioRenderer/*.cpp
	Camera/*.cpp
	CaptureSession/*.cpp
	Microphone/*.cpp
	RendererSession/*.cpp
	UI/*.cpp
	VideoRenderer/*.cpp
	*.cpp)

find_package(PkgConfig REQUIRED)
pkg_search_module(GTKMM3 REQUIRED gtkmm-3.0)

include_directories(.
	PUBLIC ${GTKMM3_INCLUDE_DIRS})

add_executable(LinStudio ${SOURCES})

target_link_libraries(LinStudio
	Engine
	rt
	boost_system
	ssl
	crypto
	pthread
	dl
	opus
	webm
	vpx
	ippi
	ipps
	ippcc
	ippdc
	ippcore
	pulse-simple
	pulse
	asound
	X11
	mp3lame
	${GTKMM3_LIBRARIES}
)
