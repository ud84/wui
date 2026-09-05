file(GLOB SOURCES 
	*.cpp
	ButtonSheet/*.cpp
	InputSheet/*.cpp
	MainFrame/*.cpp
	MainSheet/*.cpp)

include_directories(.)

add_executable(demo ${SOURCES})

target_link_libraries(demo PRIVATE wui)
if(APPLE)
    set_target_properties(demo PROPERTIES MACOSX_BUNDLE TRUE
        MACOSX_BUNDLE_GUI_IDENTIFIER "org.libwui.demo")
    add_custom_command(TARGET demo POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${CMAKE_CURRENT_SOURCE_DIR}/res" "$<TARGET_BUNDLE_CONTENT_DIR:demo>/Resources/res")
elseif(NOT EMSCRIPTEN)
    add_custom_command(TARGET demo POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${CMAKE_CURRENT_SOURCE_DIR}/res" "$<TARGET_FILE_DIR:demo>/res")
endif()

if(EMSCRIPTEN)
    wui_wasm_app(demo "${CMAKE_CURRENT_SOURCE_DIR}/res")
endif()
