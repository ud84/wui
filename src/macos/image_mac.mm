// Copyright (c) 2026 Intent Garden Org. Boost Software License 1.0.
#import <Cocoa/Cocoa.h>
#import <ImageIO/ImageIO.h>
#include "image_mac.hpp"
#include <wui/system/path_tools.hpp>
#include <filesystem>

void load_image_from_data(const std::vector<uint8_t>& data, void **image)
{
    *image=nullptr;
    if (data.empty()) return;
    CFDataRef bytes=CFDataCreate(nullptr,data.data(),data.size());
    CGImageSourceRef source=CGImageSourceCreateWithData(bytes,nullptr);
    if (source) { *image=CGImageSourceCreateImageAtIndex(source,0,nullptr); CFRelease(source); }
    CFRelease(bytes);
}
void load_image_from_file(std::string_view file, std::string_view path, void **image, wui::error& err)
{
    @autoreleasepool {
        auto full=wui::real_path((std::filesystem::path(path)/std::filesystem::path(file)).string());
        NSString *name=[[NSString alloc] initWithBytes:full.data() length:full.size() encoding:NSUTF8StringEncoding];
        NSURL *url=[NSURL fileURLWithPath:name];
        CGImageSourceRef source=CGImageSourceCreateWithURL((__bridge CFURLRef)url,nullptr);
        *image=source ? CGImageSourceCreateImageAtIndex(source,0,nullptr) : nullptr;
        if (source) CFRelease(source);
        if (!*image) err={wui::error_type::file_not_found,"image::load_image_from_file()",full};
        else err.reset();
    }
}
void free_image(void **image) { if (*image) CGImageRelease(static_cast<CGImageRef>(*image)); *image=nullptr; }
int32_t mac_image_width(void *image) { return image ? static_cast<int32_t>(CGImageGetWidth(static_cast<CGImageRef>(image))) : 0; }
int32_t mac_image_height(void *image) { return image ? static_cast<int32_t>(CGImageGetHeight(static_cast<CGImageRef>(image))) : 0; }
