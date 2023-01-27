//
// Copyright (c) 2021-2022 Anton Golovkov (udattsk at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/ud84/wui
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

struct font
{
    std::string name;
    int32_t size;
    decorations decorations_;
};

}
