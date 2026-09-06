// Copyright (c) 2026 Intent Garden Org. Boost Software License 1.0.
#include "framework_wasm.hpp"
#include <emscripten.h>
namespace wui::framework
{
void framework_wasm_impl::run()
{
    running_ = true;
    // Asyncify preserves main()'s stack-owned application until stop(), while
    // yielding to DOM events and requestAnimationFrame between iterations.
    while (running_) emscripten_sleep(20);
}
void framework_wasm_impl::stop() { running_ = false; }
}
