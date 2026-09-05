// Run the existing example entry point unchanged, inspect its rendered view, then stop.
#import <Cocoa/Cocoa.h>
#include <wui/framework/framework.hpp>
#include <cstdio>
#include <filesystem>
#define main wui_example_main
#include WUI_EXAMPLE_FILE
#undef main

int main(int argc,char **argv)
{
    @autoreleasepool {
        wui::framework::init();
        std::filesystem::current_path(WUI_TEST_WORK_DIR);
        __block bool captured=false;
        NSTimer *timer=[NSTimer timerWithTimeInterval:0.4 repeats:NO block:^(NSTimer *) {
            for(NSWindow *window in NSApp.windows) {
                if(!window.visible || !window.contentView) continue;
                NSView *view=window.contentView;
                [view displayIfNeeded];
                NSBitmapImageRep *rep=[view bitmapImageRepForCachingDisplayInRect:view.bounds];
                [view cacheDisplayInRect:view.bounds toBitmapImageRep:rep];
                NSData *png=[rep representationUsingType:NSBitmapImageFileTypePNG properties:@{}];
                captured=[png writeToFile:@WUI_SCREENSHOT atomically:YES];
                break;
            }
            wui::framework::stop();
        }];
        [NSRunLoop.mainRunLoop addTimer:timer forMode:NSRunLoopCommonModes];
        int result=wui_example_main(argc,argv);
        [timer invalidate];
        if(result || !captured) { std::fprintf(stderr,"Example failed to open/render: %d\n",result); return 1; }
        std::puts("PASS: example opened, loaded bundled resources, rendered, and stopped");
        return 0;
    }
}
