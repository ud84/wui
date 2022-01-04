cmake_minimum_required (VERSION 2.8.11)

project (wui CXX C)

set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -static-libstdc++ -static-libgcc -Wno-unused-command-line-argument")

#set(CMAKE_CXX_FLAGS:STRING= -fsanitize=address  -fsanitize=leak -g)
#set(CMAKE_C_FLAGS:STRING=-fsanitize=address  -fsanitize=leak -g)
#set(CMAKE_EXE_LINKER_FLAGS:STRING=-fsanitize=address  -fsanitize=leak)
#set(CMAKE_MODULE_LINKER_FLAGS:STRING=-fsanitize=address  -fsanitize=leak)

include_directories(
	include)

#link_directories()

add_subdirectory (src)
#add_subdirectory (examples)
