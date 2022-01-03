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

template <typename T>
inline bool enum_is_set(T value, T style)
{
    return ((static_cast<int>(value)) & (static_cast<int>(style)));
}

}
