add_executable(wui_wasm_test integration.cpp)
target_link_libraries(wui_wasm_test PRIVATE wui)
target_compile_options(wui_wasm_test PRIVATE -UNDEBUG)
wui_wasm_app(wui_wasm_test "${PROJECT_SOURCE_DIR}/examples/simple/res")
