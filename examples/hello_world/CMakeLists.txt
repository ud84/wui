file(GLOB SOURCES 
	*.cpp
	MainFrame/*.cpp)

include_directories(.)

add_executable(hello_world ${SOURCES})

target_link_libraries(hello_world PRIVATE wui)
if(APPLE)
    set_target_properties(hello_world PROPERTIES MACOSX_BUNDLE TRUE
        MACOSX_BUNDLE_GUI_IDENTIFIER "org.libwui.hello_world")
    add_custom_command(TARGET hello_world POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${CMAKE_CURRENT_SOURCE_DIR}/res" "$<TARGET_BUNDLE_CONTENT_DIR:hello_world>/Resources/res")
elseif(NOT EMSCRIPTEN)
    add_custom_command(TARGET hello_world POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${CMAKE_CURRENT_SOURCE_DIR}/res" "$<TARGET_FILE_DIR:hello_world>/res")
endif()

if(EMSCRIPTEN)
    wui_wasm_app(hello_world "${CMAKE_CURRENT_SOURCE_DIR}/res")
endif()
