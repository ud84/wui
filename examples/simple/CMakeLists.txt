file(GLOB SOURCES 
	*.cpp)

include_directories(.)

add_executable(simple ${SOURCES})

target_link_libraries(simple PRIVATE wui)
if(APPLE)
    set_target_properties(simple PROPERTIES MACOSX_BUNDLE TRUE
        MACOSX_BUNDLE_GUI_IDENTIFIER "org.libwui.simple")
    add_custom_command(TARGET simple POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${CMAKE_CURRENT_SOURCE_DIR}/res" "$<TARGET_BUNDLE_CONTENT_DIR:simple>/Resources/res")
elseif(NOT EMSCRIPTEN)
    add_custom_command(TARGET simple POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${CMAKE_CURRENT_SOURCE_DIR}/res" "$<TARGET_FILE_DIR:simple>/res")
endif()

if(EMSCRIPTEN)
    wui_wasm_app(simple "${CMAKE_CURRENT_SOURCE_DIR}/res")
endif()
