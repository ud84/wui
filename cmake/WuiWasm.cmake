# Configure a browser executable and its preloaded virtual /res directory.
function(wui_wasm_app target resource_dir)
    set_target_properties(${target} PROPERTIES SUFFIX ".html")
    target_link_options(${target} PRIVATE
        "SHELL:--shell-file ${PROJECT_SOURCE_DIR}/src/wasm/shell.html"
        "SHELL:--preload-file ${resource_dir}@/res")
    file(GLOB_RECURSE resources CONFIGURE_DEPENDS "${resource_dir}/*")
    set_property(TARGET ${target} APPEND PROPERTY LINK_DEPENDS
        "${PROJECT_SOURCE_DIR}/src/wasm/shell.html" ${resources})
endfunction()

# A self-contained directory for a static host, without CMake build files.
function(wui_wasm_site)
    set(site "${PROJECT_BINARY_DIR}/site")
    add_custom_target(wui_web_site
        COMMAND ${CMAKE_COMMAND} -E make_directory "${site}"
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${PROJECT_SOURCE_DIR}/src/wasm/index.html" "${site}/index.html"
        DEPENDS hello_world simple demo)
    foreach(app hello_world simple demo)
        add_custom_command(TARGET wui_web_site POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E make_directory "${site}/${app}")
        foreach(extension html js wasm data)
            add_custom_command(TARGET wui_web_site POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy_if_different
                    "$<TARGET_FILE_DIR:${app}>/${app}.${extension}" "${site}/${app}/${app}.${extension}")
        endforeach()
    endforeach()
endfunction()
