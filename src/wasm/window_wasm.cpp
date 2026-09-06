// Copyright (c) 2026 Intent Garden Org. Boost Software License 1.0.
#include "window_wasm.hpp"
#include <wui/control/input.hpp>
#include <wui/theme/theme.hpp>
#include <wui/system/wm_tools.hpp>
#include <wui/common/flag_helpers.hpp>
#include <emscripten.h>
#include <algorithm>
#include <map>
#include <cstring>

EM_JS(int, browser_create, (int x,int y,int width,int height), { return Module.wui.create(x,y,width,height); });
EM_JS(void, browser_close, (int id), { Module.wui.close(id); });
EM_JS(void, browser_invalidate, (int id), { Module.wui.invalidate(id); });
EM_JS(void, browser_position, (int id,int x,int y,int width,int height), { Module.wui.position(id,x,y,width,height); });
EM_JS(void, browser_style, (int id,const char *title,int minw,int minh), {
    var w=Module.wui.windows.get(id); if(w) { w.title=UTF8ToString(title); w.canvas.setAttribute('aria-label',w.title); w.minw=Math.max(40,minw); w.minh=Math.max(40,minh); }
});
EM_JS(void, browser_show, (int id,int visible), { Module.wui.show(id,visible); });
EM_JS(void, browser_minimize, (int id), { Module.wui.minimize(id); });
EM_JS(void, browser_input_position, (int id,int x,int y,int width,int height,int input), {
    var w=Module.wui.windows.get(id); if(!w) return;
    w.input=input; w.editor.style.left=x+'px'; w.editor.style.top=y+'px';
    w.editor.style.width=Math.max(1,width)+'px'; w.editor.style.height=Math.max(20,height)+'px';
});
EM_JS(double, browser_scale, (), { return Math.max(1,devicePixelRatio || 1); });

namespace { std::map<int,std::weak_ptr<wui::window>> windows; }
void center_horizontally(wui::rect& p,wui::system_context& c) { p.left=std::max(0,(wui::get_screen_size(c).width()-p.right)/2); p.right+=p.left; }
void center_vertically(wui::rect& p,wui::system_context& c) { p.top=std::max(0,(wui::get_screen_size(c).height()-p.bottom)/2); p.bottom+=p.top; }
namespace wui
{
bool wasm_window_backend::create(window& w)
{
    if(w.context_.valid()) { w.err={error_type::already_started,"window::init()","Window already exists"}; return false; }
    if(w.position_.left==-1) center_horizontally(w.position_,w.context_);
    if(w.position_.top==-1) center_vertically(w.position_,w.context_);
    if(w.position_.width()<=0 || w.position_.height()<=0) { w.err={error_type::invalid_value,"window::init()","Invalid dimensions"}; return false; }
    w.context_.canvas=browser_create(w.position_.left,w.position_.top,w.position_.width(),w.position_.height());
    w.context_.scale=browser_scale();
    windows[w.context_.canvas]=w.weak_from_this();
    style(w);
    if(!w.graphic_.init({0,0,w.position_.width(),w.position_.height()},theme_color(w.tcn,window::tv_background,w.theme_))) {
        w.err=w.graphic_.get_error(); close(w,false); return false;
    }
    w.send_internal(internal_event_type::window_created,0,0);
    if(!w.context_.valid()) return true;
    w.send_internal(internal_event_type::size_changed,w.position_.width(),w.position_.height());
    if(w.context_.valid()) show(w,w.showed_);
    invalidate(w,{}); return true;
}
void wasm_window_backend::close(window& w,bool notify)
{
    if(!w.context_.valid()) return;
    int id=w.context_.canvas; w.context_.canvas=0; windows.erase(id);
    browser_close(id); w.graphic_.release();
    if(auto parent=w.get_transient_window()) parent->enable();
    if(notify && w.close_callback) { auto callback=w.close_callback; callback(); }
}
void wasm_window_backend::invalidate(window& w,rect) { if(w.context_.valid()) browser_invalidate(w.context_.canvas); }
void wasm_window_backend::position(window& w,rect p)
{
    if(!w.context_.valid()) return;
    if(p.left==-1) center_horizontally(p,w.context_);
    if(p.top==-1) center_vertically(p,w.context_);
    if(p.width()<=0 || p.height()<=0) return;
    bool moved=w.position_.left!=p.left || w.position_.top!=p.top;
    browser_position(w.context_.canvas,p.left,p.top,p.width(),p.height());
    w.position_=p; resized(w,p.width(),p.height(),browser_scale());
    if(moved) w.send_internal(internal_event_type::position_changed,p.left,p.top);
}
void wasm_window_backend::style(window& w) { if(w.context_.valid()) browser_style(w.context_.canvas,w.caption.c_str(),w.min_width,w.min_height); }
void wasm_window_backend::show(window& w,bool visible) { if(w.context_.valid()) browser_show(w.context_.canvas,visible); }
void wasm_window_backend::minimize(window& w) { if(w.context_.valid()) browser_minimize(w.context_.canvas); }
void wasm_window_backend::expand(window& w)
{
    auto screen=get_screen_size(w.context_); position(w,screen);
    w.send_internal(internal_event_type::window_expanded,screen.width(),screen.height());
}
void wasm_window_backend::restore(window& w) { show(w,true); }
void wasm_window_backend::emit(window& w,int32_t x,int32_t y)
{
    struct pending { std::weak_ptr<window> owner; int32_t x,y; };
    auto data=new pending{w.weak_from_this(),x,y};
    emscripten_async_call([](void *p) { std::unique_ptr<pending> call(static_cast<pending *>(p));
        if(auto owner=call->owner.lock(); owner && owner->context_.valid()) owner->send_internal(internal_event_type::user_emitted,call->x,call->y);
    },data,0);
}
void wasm_window_backend::paint(window& w)
{
    if(w.skip_draw_ || !w.context_.valid() || !w.showed_) return;
    rect dirty{0,0,w.position_.width(),w.position_.height()};
    w.graphic_.clear(); w.draw_caption(w.graphic_,dirty);
    auto controls=w.controls;
    for(bool topmost : {false,true}) for(auto& c:controls)
        if(c && c->showed() && c->topmost()==topmost && c->position().in(dirty)) c->draw(w.graphic_,dirty);
    w.draw_border(w.graphic_); w.graphic_.flush(dirty); input_position(w);
}
void wasm_window_backend::resized(window& w,int width,int height,double scale)
{
    if(!w.context_.valid()) return;
    bool changed=w.position_.width()!=width || w.position_.height()!=height;
    w.context_.scale=scale;
    if(w.graphic_.max_size().width()!=width || w.graphic_.max_size().height()!=height) {
        w.graphic_.release();
        if(!w.graphic_.init({0,0,width,height},theme_color(w.tcn,window::tv_background,w.theme_))) w.err=w.graphic_.get_error();
        changed=true;
    }
    w.position_.right=w.position_.left+width; w.position_.bottom=w.position_.top+height;
    w.update_buttons();
    if(changed) w.send_internal(internal_event_type::size_changed,width,height);
    invalidate(w,{});
}
int wasm_window_backend::flags(window& w,int x,int y)
{
    return ((w.enabled_ || w.docked_control) ? 1 : 0) |
        ((w.enabled_ && flag_is_set(w.window_style_,window_style::resizable)) ? 2 : 0) |
        ((w.enabled_ && flag_is_set(w.window_style_,window_style::moving)) ? 4 : 0) |
        (w.check_control_here(x,y) ? 8 : 0);
}
void wasm_window_backend::input_position(window& w)
{
    auto control=w.get_focused();
    while(auto child=std::dynamic_pointer_cast<window>(control)) control=child->get_focused();
    rect p=control ? control->position() : rect{0,0,1,20};
    browser_input_position(w.context_.canvas,p.left+3,p.top+3,p.width()-6,24,
        std::dynamic_pointer_cast<input>(control) ? 1 : 0);
}
void wasm_window_backend::mouse(window& w,mouse_event e)
{
    if(e.type==mouse_event_type::leave) {
        if(w.active_control) { event ev{};ev.type=event_type::mouse;ev.mouse_event_=e;
            w.send_event_to_control(w.active_control,ev); w.active_control.reset(); }
        w.send_event_to_plains({event_type::mouse,e});
    } else if(w.enabled_ || w.docked_control) w.send_mouse_event(e);
}
void wasm_window_backend::keyboard(window& w,keyboard_event e)
{
    if(!w.enabled_ && !w.docked_control) return;
    if(auto dialog=std::dynamic_pointer_cast<window>(w.docked_control)) {
        keyboard(*dialog,e); return;
    }
    if(e.type==keyboard_event_type::down) {
        auto key=static_cast<uint8_t>(e.key[0]);
        if(key==vk_tab) {
            if(e.modifier==vk_lshift || e.modifier==vk_rshift) {
                std::vector<std::shared_ptr<i_control>> candidates;
                for(auto& c:w.controls) if(c->showed() && c->enabled() && c->focusing()) candidates.push_back(c);
                auto current=std::find(candidates.begin(),candidates.end(),w.get_focused());
                if(!candidates.empty()) w.set_focused(current==candidates.begin() || current==candidates.end()
                    ? candidates.back() : *std::prev(current));
            } else w.change_focus();
            return;
        }
        if(key==vk_return) {
            auto input=std::dynamic_pointer_cast<wui::input>(w.get_focused());
            if(!input || (input->get_input_view()!=input_view::multiline && w.default_push_control)) {
                w.execute_focused(); return;
            }
        }
    }
    event ev{}; ev.type=event_type::keyboard; ev.keyboard_event_=e;
    w.send_event_to_plains(ev);
    w.send_event_to_control(w.get_focused(),ev);
}
void wasm_window_backend::focus(window& w,bool focused)
{
    if(focused) {
        if(auto control=w.wasm_saved_focus_.lock()) w.set_focused(control);
        w.wasm_saved_focus_.reset();
    } else {
        auto control=w.get_focused();
        w.wasm_saved_focus_=control;
        if(control) {
            event e{};e.type=event_type::internal;
            e.internal_event_={internal_event_type::remove_focus,0,0};
            w.send_event_to_control(control,e);
        }
        if(w.active_control) {
            event e{};e.type=event_type::mouse;e.mouse_event_={mouse_event_type::leave,0,0,0};
            w.send_event_to_control(w.active_control,e);
            w.active_control.reset();
        }
    }
}
}
namespace {
std::shared_ptr<wui::window> lookup(int id) { auto it=windows.find(id); return it==windows.end() ? nullptr : it->second.lock(); }
}
extern "C" {
EMSCRIPTEN_KEEPALIVE void wui_viewport(int id) {
    if(auto w=lookup(id)) {
        if(w->state()==wui::window_state::maximized) wui::wasm_window_backend::expand(*w);
        else wui::wasm_window_backend::resized(*w,w->position().width(),w->position().height(),browser_scale());
    }
}
EMSCRIPTEN_KEEPALIVE void wui_paint(int id) { if(auto w=lookup(id)) wui::wasm_window_backend::paint(*w); }
EMSCRIPTEN_KEEPALIVE void wui_resize(int id,int width,int height,double scale) { if(auto w=lookup(id)) wui::wasm_window_backend::resized(*w,width,height,scale); }
EMSCRIPTEN_KEEPALIVE void wui_position(int id,int x,int y,int width,int height) { if(auto w=lookup(id)) wui::wasm_window_backend::position(*w,{x,y,x+width,y+height}); }
EMSCRIPTEN_KEEPALIVE int wui_flags(int id,int x,int y) { if(auto w=lookup(id)) return wui::wasm_window_backend::flags(*w,x,y); return 0; }
EMSCRIPTEN_KEEPALIVE void wui_focus(int id,int focused) { if(auto w=lookup(id)) wui::wasm_window_backend::focus(*w,focused); }
EMSCRIPTEN_KEEPALIVE void wui_restore(int id) { if(auto w=lookup(id)) w->normal(); }
EMSCRIPTEN_KEEPALIVE void wui_mouse(int id,int type,int x,int y,int delta) {
    if(auto w=lookup(id)) { wui::wasm_window_backend::mouse(*w,{static_cast<wui::mouse_event_type>(type),x,y,delta});
        if(w->context().valid()) wui::wasm_window_backend::input_position(*w); }
}
EMSCRIPTEN_KEEPALIVE void wui_key(int id,int type,int code,int modifier) {
    if(auto w=lookup(id)) { wui::keyboard_event e{};e.type=static_cast<wui::keyboard_event_type>(type);e.key[0]=code;e.key_size=1;e.modifier=modifier;
        wui::wasm_window_backend::keyboard(*w,e); if(w->context().valid()) wui::wasm_window_backend::input_position(*w); }
}
EMSCRIPTEN_KEEPALIVE void wui_text(int id,const char *text) {
    const auto end=reinterpret_cast<const unsigned char *>(text)+std::strlen(text);
    if(auto w=lookup(id)) for(const unsigned char *p=reinterpret_cast<const unsigned char *>(text); p<end;) {
        size_t n=*p<0x80 ? 1 : *p<0xe0 ? 2 : *p<0xf0 ? 3 : 4;
        if(static_cast<size_t>(end-p)<n) break;
        wui::keyboard_event e{}; e.type=wui::keyboard_event_type::key;e.key_size=n;std::memcpy(e.key,p,n);
        wui::wasm_window_backend::keyboard(*w,e);p+=n;
    }
}
}
