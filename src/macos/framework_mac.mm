// Copyright (c) 2026 Intent Garden Org. Boost Software License 1.0.
#import <Cocoa/Cocoa.h>
#include "framework_mac.hpp"
#include <wui/framework/framework.hpp>
#include <wui/window/window.hpp>

@interface WUIApplicationDelegate : NSObject <NSApplicationDelegate>
@end
@implementation WUIApplicationDelegate
- (NSApplicationTerminateReply)applicationShouldTerminate:(NSApplication *)sender
{
    // Respect WUI's close confirmation callbacks.
    for (NSWindow *window in [sender.windows copy])
        [window performClose:nil];
    return NSTerminateCancel;
}
@end

namespace wui::framework
{
static WUIApplicationDelegate *app_delegate;
void init_macos()
{
    @autoreleasepool {
        NSCAssert([NSThread isMainThread], @"WUI must be initialized on the main thread");
        [NSApplication sharedApplication];
        [NSApp setActivationPolicy:NSApplicationActivationPolicyRegular];
        if (!app_delegate) {
            app_delegate = [WUIApplicationDelegate new];
            NSApp.delegate = app_delegate;
            NSMenu *menu = [NSMenu new];
            NSMenuItem *item = [NSMenuItem new];
            [menu addItem:item];
            NSMenu *application = [NSMenu new];
            [application addItemWithTitle:@"Quit" action:@selector(terminate:) keyEquivalent:@"q"];
            item.submenu = application;
            NSApp.mainMenu = menu;
        }
        [NSApp finishLaunching];
    }
}
void framework_mac_impl::run()
{
    @autoreleasepool {
        if (![NSThread isMainThread]) {
            err_ = {error_type::system_error, "framework::run()", "AppKit requires the main thread"};
            return;
        }
        started_ = true;
        [NSApp activateIgnoringOtherApps:YES];
        [NSApp run];
        started_ = false;
    }
}
void framework_mac_impl::stop()
{
    // stop() returns control to the caller of run(), without terminating the process.
    [NSApp stop:nil];
    [NSApp postEvent:[NSEvent otherEventWithType:NSEventTypeApplicationDefined
        location:NSZeroPoint modifierFlags:0 timestamp:0 windowNumber:0 context:nil
        subtype:0 data1:0 data2:0] atStart:YES];
}
bool framework_mac_impl::started() const { return started_; }
error framework_mac_impl::get_error() const { return err_; }
}
