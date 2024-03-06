//
// Copyright (c) 2021-2024 Anton Golovkov (udattsk at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/ud84/wui
//

#include <wui/common/to_string.hpp>

namespace wui
{

std::string to_string(const rect &r)
{
    return "{" + std::to_string(r.left) + ", " + 
        std::to_string(r.top) + ", " +
        std::to_string(r.right) + ", " +
        std::to_string(r.bottom) + "}";
}

std::string to_string(const color &c)
{
    return "RGBA(" + std::to_string(get_red(c)) + "," +
        std::to_string(get_green(c)) + "," +
        std::to_string(get_blue(c)) + "," +
        std::to_string(get_alpha(c)) + ")";
}

}
