//
// Copyright (c) 2021-2022 Anton Golovkov (udattsk at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/ud84/wui
//

#pragma once

#include <wui/system/system_context.hpp>
#include <wui/common/color.hpp>
#include <wui/common/rect.hpp>
#include <string>
#include <cstdint>

namespace wui
{

enum class font_decorations : uint32_t
{
    italic = (1 << 0),
    underline = (1 << 1),
    strikeOut = (1 << 2)
};

struct font_settings
{
    std::wstring name;
    uint32_t size;
    font_decorations decorations;
};

class graphic
{
public:
	/// base context
    graphic(system_context &context);

    /// create another compatible graphic
    graphic(graphic &graphic_, rect size);

    ~graphic();

    void draw_line(const rect &position, color color_, uint32_t width = 1);

    void draw_text(const rect &position, const std::wstring &text, color color_, const font_settings &font);

    void draw_rect(const rect &position, color fill_color);
    void draw_rect(const rect &position, color border_color, color fill_color, uint32_t round);

    /// draw some buffer on context
    void draw_buffer(const rect &position, uint8_t *buffer, size_t buffer_size);

    /// draw another graphic on context
    void draw_graphic(const rect &position, graphic &graphic_);

    system_context &context();

private:
    system_context &context_;
};

}
