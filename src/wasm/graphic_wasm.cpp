// Copyright (c) 2026 Intent Garden Org. Boost Software License 1.0.
#include <wui/graphic/graphic.hpp>
#include "image_wasm.hpp"
#include <emscripten.h>
#include <algorithm>
#include <cmath>
#include <map>
#include <tuple>

EM_JS(int, canvas_create, (int width, int height, double scale), { return Module.wui.surface(width, height, scale); });
EM_JS(void, canvas_free, (int id), { Module.wui.surfaces.delete(id); });
EM_JS(void, canvas_draw, (int id, int op, double x, double y, double w, double h, unsigned color, unsigned fill, double line, double radius), {
    Module.wui.draw(id, op, x, y, w, h, color, fill, line, radius);
});
EM_JS(double, canvas_text, (int id, const char *text, int length, const char *font, int size, int decoration, int x, int y, unsigned color), {
    return Module.wui.text(id, UTF8ToString(text, length), UTF8ToString(font), size, decoration, x, y, color);
});
EM_JS(void, canvas_blit, (int id, int src, int x, int y, int width, int height, int sx, int sy), {
    Module.wui.blit(id, src, x, y, width, height, sx, sy);
});
EM_JS(void, canvas_image, (int id, int image, int x, int y, int width, int height), {
    var ctx = Module.wui.surfaces.get(id), img = Module.wui.images.get(image);
    if(ctx && img && img.complete && img.naturalWidth) ctx.drawImage(img, x, y, width, height);
});
EM_JS(void, canvas_pixels, (int id, const uint8_t *data, int x, int y, int width, int height, int sx, int sy), {
    Module.wui.pixels(id, data, x, y, width, height, sx, sy);
});
EM_JS(void, canvas_present, (int id, int window), { Module.wui.present(id, window); });

namespace wui
{
struct graphic::wasm_state
{
    int canvas = 0;
    double scale = 1;
    std::unordered_map<std::string, void *> images;
    ~wasm_state() { if(canvas) canvas_free(canvas); for(auto& entry : images) free_image(&entry.second); }
};
primitive_container::primitive_container(system_context& c) : context_(c) {}
primitive_container::~primitive_container() = default;
void primitive_container::init() {}
void primitive_container::release() {}
error primitive_container::get_error() const { return err; }
graphic::graphic(system_context& c) : context_(c), pc(c), max_size_{}, background_color(0), wasm_(new wasm_state) {}
graphic::~graphic() = default;
bool graphic::init(rect size, color background)
{
    if(wasm_->canvas) { err = {error_type::already_started,"graphic::init()","Graphics already initialized"}; return false; }
    if(size.width() <= 0 || size.height() <= 0) { err = {error_type::invalid_value,"graphic::init()","Invalid dimensions"}; return false; }
    wasm_->scale = std::max(1.0, context_.scale);
    wasm_->canvas = canvas_create(size.width(), size.height(), wasm_->scale);
    if(!wasm_->canvas) { err = {error_type::no_handle,"graphic::init()","Canvas allocation failed"}; return false; }
    max_size_ = {0,0,size.width(),size.height()}; background_color = background; err.reset(); clear(); return true;
}
void graphic::release() { wasm_.reset(new wasm_state); max_size_ = {}; }
rect graphic::max_size() const { return wasm_->scale == std::max(1.0,context_.scale) ? max_size_ : rect{}; }
int graphic::drawable() const { return wasm_->canvas; }
void graphic::set_background_color(color c) { background_color = c; clear(); }
void graphic::clear(rect r) { if(r.is_null()) r = max_size_; canvas_draw(drawable(),0,r.left,r.top,r.width(),r.height(),background_color,0,0,0); }
void graphic::flush(rect) { if(context_.valid()) canvas_present(drawable(),context_.canvas); }
void graphic::draw_pixel(rect p,color c) { draw_rect({p.left,p.top,p.left+1,p.top+1},c); }
void graphic::draw_line(rect p,color c,uint32_t width) { canvas_draw(drawable(),1,p.left,p.top,p.right,p.bottom,c,0,width,0); }
void graphic::draw_rect(rect p,color c) { canvas_draw(drawable(),2,p.left,p.top,p.width(),p.height(),c,0,0,0); }
void graphic::draw_rect(rect p,color border,color fill,uint32_t width,uint32_t radius) { canvas_draw(drawable(),3,p.left,p.top,p.width(),p.height(),border,fill,width,radius); }
static rect text_size(std::string_view text,const font& f)
{
    using key = std::tuple<std::string,int32_t,decorations,std::string>;
    static std::map<key,rect> cache;
    key k{f.name,f.size,f.decorations_,std::string(text)};
    auto it=cache.find(k); if(it!=cache.end()) return it->second;
    rect result{0,0,static_cast<int32_t>(std::ceil(canvas_text(0,text.data(),text.size(),f.name.c_str(),f.size,static_cast<int>(f.decorations_),0,0,0))), std::max(1,f.size)};
    if(cache.size()>=4096) cache.clear(); cache.emplace(std::move(k),result); return result;
}
rect graphic::measure_text(std::string_view text,const font& f) { return text_size(text,f); }
void graphic::draw_text(rect p,std::string_view text,color c,const font& f) { if(drawable()) canvas_text(drawable(),text.data(),text.size(),f.name.c_str(),f.size,static_cast<int>(f.decorations_),p.left,p.top,c); }
void graphic::draw_graphic(rect p,graphic& src,int32_t x,int32_t y) { canvas_blit(drawable(),src.drawable(),p.left,p.top,p.right,p.bottom,x,y); }
void graphic::draw_buffer(rect p,uint8_t *data,int32_t x,int32_t y) { if(data && p.width()>0 && p.height()>0) canvas_pixels(drawable(),data,p.left,p.top,p.width(),p.height(),x,y); }
void graphic::draw_native_image(void *image,rect p) { if(auto img=static_cast<wasm_image *>(image)) canvas_image(drawable(),img->id,p.left,p.top,p.width(),p.height()); }
void graphic::draw_image(std::string_view file,rect p)
{
    auto it=wasm_->images.find(std::string(file));
    if(it==wasm_->images.end()) {
        void *img=nullptr; load_image_from_file(file,{},&img,err); if(!img) return;
        it=wasm_->images.emplace(std::string(file),img).first;
    }
    draw_native_image(it->second,p);
}
error graphic::get_error() const { return err; }
void init_text_measurer(graphic&) {}
rect measure_text(std::string_view text,const font& f,graphic*) { return text_size(text,f); }
}
