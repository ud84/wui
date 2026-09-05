// Copyright (c) 2026 Intent Garden Org. Boost Software License 1.0.
#pragma once
#include <wui/window/window.hpp>
void center_horizontally(wui::rect&, wui::system_context&);
void center_vertically(wui::rect&, wui::system_context&);
namespace wui
{
struct wasm_window_backend
{
    static bool create(window&);
    static void close(window&, bool notify);
    static void invalidate(window&, rect);
    static void position(window&, rect);
    static void style(window&);
    static void show(window&, bool);
    static void minimize(window&);
    static void expand(window&);
    static void restore(window&);
    static void emit(window&, int32_t, int32_t);
    static void paint(window&);
    static void resized(window&, int width, int height, double scale);
    static void mouse(window&, mouse_event);
    static void keyboard(window&, keyboard_event);
    static void focus(window&, bool);
    static int flags(window&, int x, int y);
    static void input_position(window&);
};
}
