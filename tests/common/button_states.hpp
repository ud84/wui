// Copyright (c) 2026 Intent Garden Org. Boost Software License 1.0.
#pragma once
#include <wui/control/button.hpp>
#include <wui/theme/theme.hpp>
#include <cstdio>
#ifdef __APPLE__
#include "../../src/macos/window_mac.hpp"
using button_backend = wui::macos_window_backend;
#elif __EMSCRIPTEN__
#include "../../src/wasm/window_wasm.hpp"
using button_backend = wui::wasm_window_backend;
#endif

inline bool run_button_state_tests()
{
    using namespace wui;
    static_assert(static_cast<int>(button_view::sheet)==7, "Existing view values are stable");
    auto w=std::make_shared<window>();
    // Deliberately no PNGs: built-in state indicators must work with colors/fonts alone.
    auto theme=make_custom_theme("vector-buttons", R"({"controls":[{"type":"button",
        "calm":"#008060","active":"#00a080","text":"#ffffff","disabled":"#777777",
        "focused_border":"#ff9900","focusing":1,"font":{"name":"Segoe UI","size":18}}]})");
    int callbacks=0, failures=0;
    bool callback_state=false;
    auto b=std::make_shared<button>("Vector control", []{}, button_view::text, "button", theme);
    b->set_callback([&]{++callbacks;callback_state=b->turned();});
    w->add_control(b,{20,60,300,100});
    if(!w->init("Button state tests",{20,20,360,160},window_style::frame)) return false;
    auto check=[&](bool ok,const char* name) {if(!ok) {++failures;std::fprintf(stderr,"FAIL button: %s\n",name);}};
    auto click=[&](int x) {
        button_backend::mouse(*w,{mouse_event_type::left_down,x,80,0});
        button_backend::mouse(*w,{mouse_event_type::left_up,x,80,0});
    };
    for(auto view:{button_view::switcher,button_view::checkbox,button_view::radio}) {
        auto direct=std::make_shared<button>("No image resources", []{}, view, "button", theme);
        check(direct->get_error().is_ok(),"state control constructs without images");
        direct->turn(true);direct->update_theme(theme);
        check(direct->get_error().is_ok(),"state/theme changes need no images");
        b->set_button_view(view);b->turn(false);callbacks=0;
        b->draw(w->get_graphic(),{});
        check(b->get_error().is_ok(),"runtime view change from text draws without images");
        click(30);check(b->turned()&&callbacks==1&&callback_state,"indicator click updates state before callback");
        click(160);check(!b->turned()&&callbacks==2&&!callback_state,"caption click toggles once");
        w->set_focused(b);
        keyboard_event key{};key.type=keyboard_event_type::down;key.key_size=1;key.key[0]=0x20;
        button_backend::keyboard(*w,key);
        key.type=keyboard_event_type::key;button_backend::keyboard(*w,key);
        key.type=keyboard_event_type::up;button_backend::keyboard(*w,key);
        check(b->turned()&&callbacks==3,"Space toggles once");
        key.type=keyboard_event_type::down;key.key[0]=vk_return;button_backend::keyboard(*w,key);
        key.type=keyboard_event_type::up;button_backend::keyboard(*w,key);
        check(!b->turned()&&callbacks==4,"Enter toggles once");
        key.key[0]=0x20;
        b->turn(false);check(callbacks==4,"programmatic turn does not fire callback");
        b->disable();click(30);
        key.type=keyboard_event_type::down;button_backend::keyboard(*w,key);
        key.type=keyboard_event_type::key;button_backend::keyboard(*w,key);
        key.type=keyboard_event_type::up;button_backend::keyboard(*w,key);
        check(!b->turned()&&callbacks==4,"disabled control ignores input");
        b->enable();
        theme->set_dimension("button","indicator_size",32);
        b->set_position({20,60,300,110});b->update_theme(theme);b->draw(w->get_graphic(),{});
        check(b->position().height()>=38,"theme controls indicator size");
        theme->set_dimension("button","indicator_size",0);
        b->set_position({20,60,300,100});
    }
    b->set_callback({}); // callback borrows local counters
    w->destroy();w->remove_control(b);
    if(!failures) std::puts("PASS: vector button states, callbacks, keyboard, disabled state, custom themes and runtime views");
    return failures==0;
}
