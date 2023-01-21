//
// Copyright (c) 2021-2023 Anton Golovkov (udattsk at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/ud84/wui
//

#include <wui/graphic/primitive_container.hpp>

#include <wui/common/flag_helpers.hpp>

#include <boost/nowide/convert.hpp>

namespace wui
{

#ifdef _WIN32

void primitive_container::init()
{
}

void primitive_container::release()
{
    for (auto &p : pens)
    {
        DeleteObject(p.second);
    }
    pens.clear();

    for (auto &b : brushes)
    {
        DeleteObject(b.second);
    }
    brushes.clear();

    for (auto &f : fonts)
    {
        DeleteObject(f.second);
    }
    fonts.clear();
}

HPEN primitive_container::get_pen(int32_t style, int32_t width, color color_)
{
    auto it = pens.find({ style, width, color_ });
    if (it != pens.end())
    {
        return it->second;
    }
    auto pen = CreatePen(PS_SOLID, width, color_);

    pens[{ style, width, color_ }] = pen;

    return pen;
}

HBRUSH primitive_container::get_brush(color color_)
{
    auto it = brushes.find(color_);
    if (it != brushes.end())
    {
        return it->second;
    }
    auto brush = CreateSolidBrush(color_);

    brushes[color_] = brush;

    return brush;
}

HFONT primitive_container::get_font(font font_)
{
    auto it = fonts.find(font_);
    if (it != fonts.end())
    {
        return it->second;
    }

    LOGFONTW log_font = { font_.size,
        0,
        0,
        0,
        flag_is_set(font_.decorations_, decorations::bold) ? FW_MEDIUM : FW_DONTCARE,
        flag_is_set(font_.decorations_, decorations::italic),
        flag_is_set(font_.decorations_, decorations::underline),
        flag_is_set(font_.decorations_, decorations::strike_out),
        ANSI_CHARSET,
        OUT_TT_PRECIS,
        CLIP_DEFAULT_PRECIS,
        CLEARTYPE_QUALITY,
        DEFAULT_PITCH | FF_DONTCARE,
        0
    };
    auto font_name = boost::nowide::widen(font_.name);
    memcpy(log_font.lfFaceName, font_name.c_str(), font_name.size() * 2);
    HFONT font__ = CreateFontIndirectW(&log_font);

    fonts[font_] = font__;

    return font__;
}

#elif __linux__


void primitive_container::init()
{
}

void primitive_container::release()
{
}

#endif

}
