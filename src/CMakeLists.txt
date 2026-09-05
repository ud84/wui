file(GLOB SOURCES CONFIGURE_DEPENDS
    common/*.cpp control/*.cpp config/*.cpp event/*.cpp framework/*.cpp
    graphic/*.cpp theme/*.cpp locale/*.cpp system/*.cpp window/*.cpp)
if(APPLE)
    list(REMOVE_ITEM SOURCES
        "${CMAKE_CURRENT_SOURCE_DIR}/graphic/graphic.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/graphic/primitive_container.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/system/udev_handler.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/window/listener.cpp")
    file(GLOB MAC_SOURCES CONFIGURE_DEPENDS macos/*.mm)
    list(APPEND SOURCES ${MAC_SOURCES})
endif()
add_library(wui STATIC ${SOURCES})
target_compile_features(wui PUBLIC cxx_std_17)
target_include_directories(wui PUBLIC ../include PRIVATE . ../thirdparty)
if(APPLE)
    set_target_properties(wui PROPERTIES OBJCXX_STANDARD 17 OBJCXX_STANDARD_REQUIRED YES)
    target_compile_options(wui PRIVATE "$<$<COMPILE_LANGUAGE:OBJCXX>:-fobjc-arc>")
    target_link_libraries(wui PUBLIC "-framework Cocoa" "-framework CoreGraphics"
        "-framework CoreText" "-framework ImageIO")
elseif(UNIX)
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(CAIRO REQUIRED IMPORTED_TARGET cairo)
    find_package(Threads REQUIRED)
    target_link_libraries(wui PUBLIC PkgConfig::CAIRO xcb xcb-cursor xcb-ewmh
        xcb-icccm xcb-image X11 X11-xcb Threads::Threads udev)
    if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
        target_link_options(wui INTERFACE -static-libstdc++ -static-libgcc)
    endif()
    if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" AND CMAKE_CXX_COMPILER_VERSION VERSION_LESS 9)
        target_link_libraries(wui PUBLIC stdc++fs)
    endif()
endif()
