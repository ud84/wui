//
// Copyright (c) 2021-2026 Intent Garden Org
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
//

#pragma once

#include <cstdint>
#include <string>

namespace wui
{

enum class decorations : uint32_t
{
    normal = 0,
    bold = (1 << 0),
    italic = (1 << 1),
    underline = (1 << 2),
    strike_out = (1 << 3)
};


inline constexpr decorations operator|(const decorations l, const decorations r)
{
    return static_cast <decorations> (static_cast <uint32_t> (l) | static_cast <uint32_t> (r));
}

inline constexpr bool operator&(const decorations l, const decorations r)
{
    return 0 != (static_cast <uint32_t> (l) & static_cast <uint32_t> (r));
}


struct font
{
    std::string name;
    int32_t size{ };
    decorations decorations_{ decorations::normal };
};

}
