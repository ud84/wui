//
// Copyright (c) 2021-2022 Anton Golovkov (udattsk at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/ud84/WUI
//

#pragma once

#ifdef _WIN32
#include <windows.h>
#endif

namespace wui
{

struct graphic
{
#ifdef _WIN32
    HDC dc;
#endif
};

}
