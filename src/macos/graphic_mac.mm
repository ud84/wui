// Copyright (c) 2026 Intent Garden Org. Boost Software License 1.0.
#import <Cocoa/Cocoa.h>
#import <ImageIO/ImageIO.h>
#include <wui/graphic/graphic.hpp>
#include <wui/system/path_tools.hpp>
#include <wui/common/flag_helpers.hpp>
#include <algorithm>
#include <cmath>
#include <map>
#include <tuple>

namespace wui
{
struct graphic::mac_state
{
    CGContextRef context = nullptr;
    double scale = 1;
    std::unordered_map<std::string, CGImageRef> images;
    ~mac_state() {
        for (auto& entry : images) CGImageRelease(entry.second);
        if (context) CGContextRelease(context);
    }
};

// AppKit font measurement needs no window or mutable global graphics context.
static NSDictionary *attributes(const font& f, color c)
{
    NSString *name = [[NSString alloc] initWithBytes:f.name.data() length:f.name.size() encoding:NSUTF8StringEncoding];
    NSFont *native = [NSFont fontWithName:name size:std::max(1, f.size)];
    if (!native) native = [NSFont systemFontOfSize:std::max(1, f.size)];
    NSFontTraitMask traits = 0;
    if (flag_is_set(f.decorations_, decorations::bold)) traits |= NSBoldFontMask;
    if (flag_is_set(f.decorations_, decorations::italic)) traits |= NSItalicFontMask;
    native = [NSFontManager.sharedFontManager convertFont:native toHaveTrait:traits];
    // WUI input/list use font.size as the line height, rather than the em size.
    CGFloat height = native.ascender - native.descender + native.leading;
    if (height > 0) native = [NSFontManager.sharedFontManager convertFont:native
        toSize:native.pointSize * std::max(1, f.size) / height];
    return @{
        NSFontAttributeName: native,
        NSForegroundColorAttributeName: [NSColor colorWithSRGBRed:get_red(c)/255.0 green:get_green(c)/255.0
            blue:get_blue(c)/255.0 alpha:get_alpha(c)/255.0],
        NSUnderlineStyleAttributeName: @(flag_is_set(f.decorations_, decorations::underline) ? NSUnderlineStyleSingle : 0),
        NSStrikethroughStyleAttributeName: @(flag_is_set(f.decorations_, decorations::strike_out) ? NSUnderlineStyleSingle : 0)
    };
}
static NSString *text_string(std::string_view s)
{
    return [[NSString alloc] initWithBytes:s.data() length:s.size() encoding:NSUTF8StringEncoding] ?: @"";
}
static rect text_size(std::string_view text, const font& f)
{
    @autoreleasepool {
        if (text.empty()) return {0, 0, 0, f.size};
        using key = std::tuple<std::string, int32_t, decorations, std::string>;
        static thread_local std::map<key, rect> cache;
        key k{f.name, f.size, f.decorations_, std::string(text)};
        auto found = cache.find(k);
        if (found != cache.end()) return found->second;
        NSSize size = [text_string(text) sizeWithAttributes:attributes(f, make_color(0,0,0))];
        rect result{0, 0, static_cast<int32_t>(std::ceil(size.width)), static_cast<int32_t>(std::ceil(size.height))};
        if (cache.size() >= 4096) cache.clear();
        cache.emplace(std::move(k), result);
        return result;
    }
}
static CGRect cg_rect(rect r) { return CGRectMake(r.left, r.top, r.width(), r.height()); }
static void fill_color(CGContextRef c, color v) {
    CGContextSetRGBFillColor(c, get_red(v)/255.0, get_green(v)/255.0, get_blue(v)/255.0, get_alpha(v)/255.0);
}
static void stroke_color(CGContextRef c, color v) {
    CGContextSetRGBStrokeColor(c, get_red(v)/255.0, get_green(v)/255.0, get_blue(v)/255.0, get_alpha(v)/255.0);
}

// There are no GDI/XCB primitives to manage on macOS.
primitive_container::primitive_container(system_context& c) : context_(c) {}
primitive_container::~primitive_container() = default;
void primitive_container::init() {}
void primitive_container::release() {}
error primitive_container::get_error() const { return err; }

graphic::graphic(system_context& c) : context_(c), pc(c), max_size_{}, background_color(0), mac_(new mac_state) {}
graphic::~graphic() = default;
bool graphic::init(rect size, color background)
{
    if (mac_->context) {
        err = {error_type::already_started, "graphic::init()", "Graphics buffer is already initialized"};
        return false;
    }
    if (size.width() <= 0 || size.height() <= 0) {
        err = {error_type::invalid_value, "graphic::init()", "Graphics buffer dimensions must be positive"};
        return false;
    }
    mac_->scale = std::max(1.0, context_.scale);
    size_t width = static_cast<size_t>(std::ceil(size.width()*mac_->scale));
    size_t height = static_cast<size_t>(std::ceil(size.height()*mac_->scale));
    CGColorSpaceRef space = CGColorSpaceCreateWithName(kCGColorSpaceSRGB);
    mac_->context = CGBitmapContextCreate(nullptr, width, height, 8, 0, space,
        kCGImageAlphaPremultipliedFirst | kCGBitmapByteOrder32Little);
    CGColorSpaceRelease(space);
    if (!mac_->context) {
        err = {error_type::no_handle, "graphic::init()", "Could not allocate a Core Graphics bitmap"};
        return false;
    }
    max_size_ = {0, 0, size.width(), size.height()};
    background_color = background;
    CGContextTranslateCTM(mac_->context, 0, height);
    CGContextScaleCTM(mac_->context, mac_->scale, -mac_->scale);
    err.reset();
    clear();
    return true;
}
void graphic::release() { mac_.reset(new mac_state); max_size_ = {}; }
rect graphic::max_size() const
{
    // Shared input/list buffers must be recreated after a Retina scale change.
    return mac_->context && mac_->scale != std::max(1.0, context_.scale) ? rect{} : max_size_;
}
void *graphic::drawable() { return mac_->context; }
void graphic::set_background_color(color c) { background_color = c; clear(); }
void graphic::clear(rect r)
{
    if (!mac_->context) return;
    CGContextSaveGState(mac_->context);
    CGContextSetBlendMode(mac_->context, kCGBlendModeCopy);
    fill_color(mac_->context, background_color);
    CGContextFillRect(mac_->context, cg_rect(r.is_null() ? max_size_ : r));
    CGContextRestoreGState(mac_->context);
}
void graphic::flush(rect r)
{
    NSView *view = (__bridge NSView *)context_.native_view;
    if (view) [view setNeedsDisplayInRect:cg_rect(r)];
}
void graphic::draw_pixel(rect p, color c) { draw_rect({p.left,p.top,p.left+1,p.top+1},c); }
void graphic::draw_line(rect p, color c, uint32_t width)
{
    if (!mac_->context || !width) return;
    auto ctx=mac_->context;
    CGContextSaveGState(ctx);
    stroke_color(ctx,c);
    CGContextSetLineWidth(ctx,width);
    CGContextBeginPath(ctx);
    CGContextMoveToPoint(ctx,p.left+0.5,p.top+0.5);
    CGContextAddLineToPoint(ctx,p.right+0.5,p.bottom+0.5);
    CGContextStrokePath(ctx);
    CGContextRestoreGState(ctx);
}
rect graphic::measure_text(std::string_view text, const font& f) { return text_size(text,f); }
void graphic::draw_text(rect position, std::string_view text, color c, const font& f)
{
    if (!mac_->context || text.empty()) return;
    @autoreleasepool {
        [NSGraphicsContext saveGraphicsState];
        NSGraphicsContext.currentContext = [NSGraphicsContext graphicsContextWithCGContext:mac_->context flipped:YES];
        [text_string(text) drawAtPoint:NSMakePoint(position.left,position.top) withAttributes:attributes(f,c)];
        [NSGraphicsContext restoreGraphicsState];
    }
}
void graphic::draw_rect(rect p, color c)
{
    if (!mac_->context) return;
    auto ctx=mac_->context;
    CGContextSaveGState(ctx);
    fill_color(ctx,c);
    CGContextFillRect(ctx,CGRectStandardize(cg_rect(p)));
    CGContextRestoreGState(ctx);
}
void graphic::draw_rect(rect p, color border, color fill, uint32_t width, uint32_t radius)
{
    if (!mac_->context || p.width()<=0 || p.height()<=0) return;
    auto ctx=mac_->context;
    CGContextSaveGState(ctx);
    CGRect bounds = CGRectInset(cg_rect(p),width/2.0,width/2.0);
    if (bounds.size.width>0 && bounds.size.height>0) {
        double r=std::min<double>(radius,std::min(bounds.size.width,bounds.size.height)/2);
        CGPathRef path=CGPathCreateWithRoundedRect(bounds,r,r,nullptr);
        if (get_alpha(fill)) { fill_color(ctx,fill); CGContextAddPath(ctx,path); CGContextFillPath(ctx); }
        if (width && get_alpha(border)) {
            stroke_color(ctx,border); CGContextSetLineWidth(ctx,width);
            CGContextAddPath(ctx,path); CGContextStrokePath(ctx);
        }
        CGPathRelease(path);
    }
    CGContextRestoreGState(ctx);
}
void graphic::draw_native_image(void *image, rect p)
{
    if (!mac_->context || !image || p.width()<=0 || p.height()<=0) return;
    auto ctx=mac_->context;
    CGContextSaveGState(ctx);
    CGContextTranslateCTM(ctx,p.left,p.bottom);
    CGContextScaleCTM(ctx,1,-1);
    CGContextDrawImage(ctx,CGRectMake(0,0,p.width(),p.height()),static_cast<CGImageRef>(image));
    CGContextRestoreGState(ctx);
}
void graphic::draw_image(std::string_view file, rect p)
{
    std::string path=real_path(file);
    auto it=mac_->images.find(path);
    if (it==mac_->images.end()) {
        @autoreleasepool {
            NSURL *url=[NSURL fileURLWithPath:text_string(path)];
            CGImageSourceRef source=CGImageSourceCreateWithURL((__bridge CFURLRef)url,nullptr);
            CGImageRef img=source ? CGImageSourceCreateImageAtIndex(source,0,nullptr) : nullptr;
            if (source) CFRelease(source);
            if (!img) { err={error_type::file_not_found,"graphic::draw_image()",path}; return; }
            it=mac_->images.emplace(path,img).first;
        }
    }
    draw_native_image(it->second,p);
}
void graphic::draw_buffer(rect p, uint8_t *buffer, int32_t x, int32_t y)
{
    if (!mac_->context || !buffer || p.width()<=0 || p.height()<=0) return;
    size_t bytes=static_cast<size_t>(p.width())*p.height()*4;
    CFDataRef data=CFDataCreate(nullptr,buffer,bytes);
    CGDataProviderRef provider=CGDataProviderCreateWithCFData(data);
    CGColorSpaceRef space=CGColorSpaceCreateWithName(kCGColorSpaceSRGB);
    CGImageRef img=CGImageCreate(p.width(),p.height(),8,32,p.width()*4,space,
        kCGImageAlphaNoneSkipFirst | kCGBitmapByteOrder32Little,provider,nullptr,false,kCGRenderingIntentDefault);
    if (img) {
        CGContextSaveGState(mac_->context);
        CGContextClipToRect(mac_->context,cg_rect(p));
        draw_native_image(img,{p.left-x,p.top-y,p.right-x,p.bottom-y});
        CGContextRestoreGState(mac_->context);
        CGImageRelease(img);
    }
    CGColorSpaceRelease(space); CGDataProviderRelease(provider); CFRelease(data);
}
void graphic::draw_graphic(rect p, graphic& source, int32_t x, int32_t y)
{
    if (!mac_->context || !source.mac_->context) return;
    CGImageRef img=CGBitmapContextCreateImage(source.mac_->context);
    if (!img) return;
    CGContextSaveGState(mac_->context);
    // Existing WUI draw_graphic callers pass {x, y, width, height}, unlike draw_rect.
    CGContextClipToRect(mac_->context,CGRectMake(p.left,p.top,p.right,p.bottom));
    draw_native_image(img,{p.left-x,p.top-y,p.left-x+source.max_size_.width(),p.top-y+source.max_size_.height()});
    CGContextRestoreGState(mac_->context);
    CGImageRelease(img);
}
error graphic::get_error() const { return err; }
void init_text_measurer(graphic&) {} // Measurement is independent of any window on macOS.
rect measure_text(std::string_view text, const font& f, graphic*) { return text_size(text,f); }
}
