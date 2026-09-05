#pragma once
#include <wui/control/tray_icon.hpp>
namespace wui {
struct macos_tray_backend {
    static void create(tray_icon& icon);
    static void release(tray_icon& icon);
    static void update(tray_icon& icon);
    static void message(tray_icon& icon,std::string_view title,std::string_view message);
};
}
