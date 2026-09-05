// Copyright (c) 2026 Intent Garden Org. Boost Software License 1.0.
#include <wui/system/clipboard_tools.hpp>
#include <wui/system/wm_tools.hpp>
#include <wui/system/tools.hpp>
#include <wui/system/uri_tools.hpp>
#include <wui/system/timer.hpp>
#include <emscripten.h>
#include <emscripten/html5.h>
#include <algorithm>
namespace { std::string clipboard; }
extern "C" EMSCRIPTEN_KEEPALIVE void wui_clipboard(const char *text) { clipboard=text; }
EM_JS(void, browser_copy, (const char *text), {
    Module.wui.clipboard=UTF8ToString(text);
    // cut/copy event handlers synchronously populate clipboardData.
    if(!Module.wui.clipboardEvent && navigator.clipboard)
        navigator.clipboard.writeText(Module.wui.clipboard).catch(function() {});
});
EM_JS(int, browser_open, (const char *text), {
    try { var url=new URL(UTF8ToString(text),location.href);
        if(!['http:','https:','mailto:'].includes(url.protocol)) return 0;
        var opened=window.open(url.href,'_blank'); if(opened) { opened.opener=null; return 1; } return 0;
    } catch(e) { return 0; }
});
EM_JS(int, browser_width, (), { return Module.wui.host().clientWidth; });
EM_JS(int, browser_height, (), { return Module.wui.host().clientHeight; });
EM_JS(void, browser_cursor, (int id,int kind), {
    var w=Module.wui.windows.get(id); if(w) w.canvas.style.cursor=['default','pointer','text','wait','nwse-resize','nesw-resize','ew-resize','ns-resize'][kind] || 'default';
});
namespace wui
{
void clipboard_put(std::string_view text,system_context&) { clipboard=text;browser_copy(clipboard.c_str()); }
bool is_text_in_clipboard(system_context&) { return !clipboard.empty(); }
std::string clipboard_get_text(system_context&) { return clipboard; }
bool open_uri(std::string_view uri) { return browser_open(std::string(uri).c_str()); }
void hide_taskbar_icon(system_context&) {}
void show_taskbar_icon(system_context&) {}
rect get_screen_size(system_context&) { return {0,0,browser_width(),browser_height()}; }
void set_cursor(system_context& c,cursor value) { browser_cursor(c.canvas,static_cast<int>(value)); }
timer::timer(std::function<void(void)> callback) : callback_(std::move(callback)) {}
timer::~timer() { stop(); }
void timer::start(uint32_t interval)
{
    if(native_timer_) return;
    auto id=emscripten_set_interval([](void *p) {
        auto callback=static_cast<timer *>(p)->callback_; callback(); // May destroy this timer.
    },std::max(1u,interval),this);
    native_timer_=reinterpret_cast<void *>(static_cast<intptr_t>(id));
}
void timer::stop()
{
    if(native_timer_) { emscripten_clear_interval(static_cast<int>(reinterpret_cast<intptr_t>(native_timer_))); native_timer_=nullptr; }
}
}
