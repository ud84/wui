//
// Copyright (c) 2021-2022 Anton Golovkov (udattsk at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/ud84/wui
//

#include <wui/graphic/graphic.hpp>

namespace wui
{

graphic::graphic(graphic_context &context__)
    : context_(context__)
{
}

graphic::graphic(graphic &graphic_, rect size)
    : context_(graphic_.context())
{
}

graphic::~graphic()
{
}

void graphic::draw_line(const rect &position, color color_, uint32_t width)
{
}

void graphic::draw_text(const rect &position, const std::wstring &text, color color_, const font_settings &font)
{
}

void graphic::draw_rect(const rect &position, color fill_color)
{
}

void graphic::draw_rect(const rect &position, color border_color, color fill_color, uint32_t round)
{
}

void graphic::draw_buffer(const rect &position, uint8_t *buffer, size_t buffer_size)
{
}

void graphic::draw_graphic(const rect &position, graphic &graphic_)
{
}

graphic_context &graphic::context()
{
    return context_;
}

}
