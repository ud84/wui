//
// Copyright (c) 2021-2022 Anton Golovkov (udattsk at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/ud84/WUI
//

#pragma once

namespace wui
{

typedef unsigned long color;

static inline color make_color(unsigned char red, unsigned char green, unsigned char blue)
{
    return ((red) | (static_cast<unsigned short>(green) << 8)) | (static_cast<unsigned long>(blue) << 16);
}

static inline char get_red(color rgb)
{
	return (rgb >> 16) & 0xFF;
}

static inline char get_green(color rgb)
{
	return (rgb >> 8) & 0xFF;
}

static inline char get_blue(color rgb)
{
	return rgb & 0xFF;
}

}
