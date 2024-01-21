//
// Copyright (c) 2021-2022 Anton Golovkov (udattsk at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/ud84/wui
//

#pragma once

namespace wui
{

typedef unsigned long color;

static inline color make_color(unsigned char red, unsigned char green, unsigned char blue)
{
#ifdef _WIN32
    return ((red) | (static_cast<unsigned short>(green) << 8)) | (static_cast<unsigned long>(blue) << 16);
#elif __linux__
    return ((blue) | (static_cast<unsigned short>(green) << 8)) | (static_cast<unsigned long>(red) << 16);
#endif
}

static inline unsigned char get_red(color rgb)
{
	return (rgb >> 16) & 0xFF;
}

static inline unsigned char get_green(color rgb)
{
	return (rgb >> 8) & 0xFF;
}

static inline unsigned char get_blue(color rgb)
{
	return rgb & 0xFF;
}

struct color_alpha
{
	color c;
	unsigned char a;

	color_alpha(color c_) : c(c_), a(255) {}
	color_alpha(color c_, unsigned char a_) : c(c_), a(a_) {}
};

}
