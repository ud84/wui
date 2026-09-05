#pragma once
#include <wui/window/window.hpp>
void center_horizontally(wui::rect& position, wui::system_context& context);
void center_vertically(wui::rect& position, wui::system_context& context);
namespace wui
{
// Private bridge: the public window remains ordinary C++.
struct macos_window_backend
{
    static bool create(window& w);
    static void close(window& w, bool notify);
    static void invalidate(window& w, rect position);
    static void position(window& w, rect position);
    static void style(window& w);
    static void show(window& w, bool visible);
    static void minimize(window& w);
    static void expand(window& w);
    static void restore(window& w);
    static void emit(window& w, int32_t x, int32_t y);
    static void paint(window& w, rect dirty, void *destination);
    static void resized(window& w);
    static void moved(window& w);
    static void mouse(window& w, mouse_event event);
    static void keyboard(window& w, keyboard_event event);
    static bool hit_control(window& w, int32_t x, int32_t y);
    static bool resizable(window& w);
    static bool accepts_input(window& w);
    static rect composition_rect(window& w);
    static bool movable(window& w);
    static void focus(window& w, bool focused);
};
}
