// Copyright (c) 2026 Intent Garden Org. Boost Software License 1.0.
#import <Cocoa/Cocoa.h>
#include "tray_mac.hpp"
#include <wui/system/path_tools.hpp>

@interface WUIStatusItem : NSObject
@property(strong) NSStatusItem *item;
@property(strong) NSPopover *popover;
@property(copy) void (^clicked)(wui::tray_icon_action);
@end
@implementation WUIStatusItem
- (void)messageClicked:(id)sender
{
    auto callback=self.clicked;
    [self.popover close];
    if(callback) callback(wui::tray_icon_action::message_click);
}
- (void)clicked:(id)sender
{
    auto callback=self.clicked;
    if(callback) {
        NSEventType type=NSApp.currentEvent.type;
        callback(type==NSEventTypeRightMouseUp ? wui::tray_icon_action::right_click :
            type==NSEventTypeOtherMouseUp ? wui::tray_icon_action::center_click : wui::tray_icon_action::left_click);
    }
}
@end
namespace wui {
static NSString *native_string(std::string_view s) {
    return [[NSString alloc] initWithBytes:s.data() length:s.size() encoding:NSUTF8StringEncoding] ?: @"";
}
void macos_tray_backend::create(tray_icon& icon) {
    WUIStatusItem *state=[WUIStatusItem new];
    state.item=[NSStatusBar.systemStatusBar statusItemWithLength:NSSquareStatusItemLength];
    state.item.button.target=state;
    state.item.button.action=@selector(clicked:);
    [state.item.button sendActionOn:NSEventMaskLeftMouseUp|NSEventMaskRightMouseUp|NSEventMaskOtherMouseUp];
    auto ptr=&icon;
    state.clicked=^(tray_icon_action action) { auto callback=ptr->click_callback; if(callback) callback(action); };
    icon.macos_state_=(__bridge_retained void *)state;
    update(icon);
}
void macos_tray_backend::release(tray_icon& icon) {
    if(!icon.macos_state_) return;
    WUIStatusItem *state=(__bridge_transfer WUIStatusItem *)icon.macos_state_;
    icon.macos_state_=nullptr;
    state.clicked=nil; [state.popover close];
    [NSStatusBar.systemStatusBar removeStatusItem:state.item];
}
void macos_tray_backend::update(tray_icon& icon) {
    WUIStatusItem *state=(__bridge WUIStatusItem *)icon.macos_state_;
    state.item.button.toolTip=native_string(icon.tip);
    NSImage *image=[[NSImage alloc] initWithContentsOfFile:native_string(real_path(icon.icon_file_name))];
    image.size=NSMakeSize(18,18);
    state.item.button.image=image;
}
void macos_tray_backend::message(tray_icon& icon,std::string_view title,std::string_view message) {
    WUIStatusItem *state=(__bridge WUIStatusItem *)icon.macos_state_;
    if(!state) return;
    [state.popover close];
    state.popover=[NSPopover new]; state.popover.behavior=NSPopoverBehaviorTransient;
    NSViewController *controller=[NSViewController new];
    controller.view=[[NSView alloc] initWithFrame:NSMakeRect(0,0,300,120)];
    NSTextField *heading=[NSTextField labelWithString:native_string(title)];
    heading.font=[NSFont boldSystemFontOfSize:13]; heading.frame=NSMakeRect(12,87,276,20);
    NSTextField *body=[NSTextField wrappingLabelWithString:native_string(message)];
    body.frame=NSMakeRect(12,12,276,70);
    [controller.view addSubview:heading]; [controller.view addSubview:body];
    [controller.view addGestureRecognizer:[[NSClickGestureRecognizer alloc]
        initWithTarget:state action:@selector(messageClicked:)]];
    state.popover.contentViewController=controller;
    [state.popover showRelativeToRect:state.item.button.bounds ofView:state.item.button preferredEdge:NSRectEdgeMinY];
}
}
