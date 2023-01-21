//
// Copyright (c) 2021-2023 Anton Golovkov (udattsk at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/ud84/wui
//

#pragma once

#include <wui/common/color.hpp>
#include <wui/common/rect.hpp>
#include <wui/common/font.hpp>

#include <map>

#ifdef _WIN32
#include <windows.h>
#elif __linux__
#endif

namespace wui
{

#ifdef _WIN32
struct pen_vals
{
    int32_t style, width;
    color color_;
};
inline bool operator<(const pen_vals &lv, const pen_vals &rv)
{
    return lv.style < rv.style && lv.width < rv.width && lv.color_ < lv.color_;
}
#endif

class primitive_container
{
public:
    void init();
    void release();

#ifdef _WIN32
    HPEN get_pen(int32_t style, int32_t width, color color_);
    HBRUSH get_brush(color color_);
    HFONT get_font(font font_);
#elif __linux__
#endif

private:
#ifdef _WIN32
    std::map<pen_vals, HPEN> pens;
    std::map<int32_t, HBRUSH> brushes;
    std::map<font, HFONT> fonts;
#elif __linux__
#endif
};

}
