// Copyright (c) 2026 Intent Garden Org. Boost Software License 1.0.
#import <Cocoa/Cocoa.h>
#include <wui/system/clipboard_tools.hpp>
#include <wui/system/wm_tools.hpp>
#include <wui/system/tools.hpp>
#include <wui/system/uri_tools.hpp>
#include <wui/system/timer.hpp>
#include <algorithm>
#include <filesystem>

namespace wui
{
static NSString *string(std::string_view s)
{
    return [[NSString alloc] initWithBytes:s.data() length:s.size() encoding:NSUTF8StringEncoding];
}
std::string macos_resource_path(std::string_view path)
{
    @autoreleasepool {
        std::filesystem::path file(path);
        if (file.is_absolute() || std::filesystem::exists(file)) return std::filesystem::absolute(file).string();
        NSString *resources = NSBundle.mainBundle.resourcePath;
        NSString *candidate = [resources stringByAppendingPathComponent:string(path)];
        if (candidate && [NSFileManager.defaultManager fileExistsAtPath:candidate]) return candidate.UTF8String;
        return std::filesystem::absolute(file).string();
    }
}
void clipboard_put(std::string_view text, system_context&)
{
    @autoreleasepool {
        NSPasteboard *board = NSPasteboard.generalPasteboard;
        [board clearContents];
        [board setString:string(text) ?: @"" forType:NSPasteboardTypeString];
    }
}
bool is_text_in_clipboard(system_context&)
{
    @autoreleasepool { return [NSPasteboard.generalPasteboard availableTypeFromArray:@[NSPasteboardTypeString]] != nil; }
}
std::string clipboard_get_text(system_context&)
{
    @autoreleasepool {
        NSString *text = [NSPasteboard.generalPasteboard stringForType:NSPasteboardTypeString];
        return text.UTF8String ?: "";
    }
}
bool open_uri(std::string_view uri)
{
    @autoreleasepool {
        NSURL *url = [NSURL URLWithString:string(uri)];
        return url && [NSWorkspace.sharedWorkspace openURL:url];
    }
}
void hide_taskbar_icon(system_context&) {} // Dock visibility belongs to the application, not an individual window.
void show_taskbar_icon(system_context&) {}
rect get_screen_size(system_context& context)
{
    @autoreleasepool {
        NSWindow *window = (__bridge NSWindow *)context.native_window;
        NSScreen *screen = window.screen ?: NSScreen.mainScreen;
        NSRect r = screen.frame;
        return {0, 0, static_cast<int32_t>(r.size.width), static_cast<int32_t>(r.size.height)};
    }
}
void set_cursor(system_context&, cursor kind)
{
    NSCursor *c = NSCursor.arrowCursor;
    switch (kind) {
        case cursor::hand: c = NSCursor.pointingHandCursor; break;
        case cursor::ibeam: c = NSCursor.IBeamCursor; break;
        case cursor::size_we: c = NSCursor.resizeLeftRightCursor; break;
        case cursor::size_ns: c = NSCursor.resizeUpDownCursor; break;
        case cursor::size_nwse: case cursor::size_nesw: c = NSCursor.crosshairCursor; break;
        default: break;
    }
    [c set];
}

timer::timer(std::function<void(void)> callback) : callback_(std::move(callback)) {}
timer::~timer() { stop(); }
void timer::start(uint32_t interval)
{
    if (native_timer_) return;
    NSCAssert([NSThread isMainThread], @"WUI timers must be used on the main thread");
    // Copy the callback: it may destroy or restart its own timer.
    auto callback = callback_;
    NSTimer *t = [NSTimer timerWithTimeInterval:std::max(1u, interval) / 1000.0 repeats:YES
        block:^(NSTimer *) { callback(); }];
    native_timer_ = (__bridge_retained void *)t;
    [NSRunLoop.mainRunLoop addTimer:t forMode:NSRunLoopCommonModes];
}
void timer::stop()
{
    if (!native_timer_) return;
    NSTimer *t = (__bridge_transfer NSTimer *)native_timer_;
    native_timer_ = nullptr;
    [t invalidate];
}
}
