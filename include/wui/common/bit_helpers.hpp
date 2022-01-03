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
inline void set_bit(T &reg, size_t bit)
{
    reg |= (1 << bit);
}

template <typename T>
inline void clear_bit(T &reg, size_t bit)
{
    reg &= (~(1 << bit));
}

template <typename T>
inline void inv_bit(T &reg, size_t bit)
{
    reg ^= (1 << bit);
}

template <typename T>
inline bool bit_is_set(T &reg, size_t bit)
{
    return ((reg & (1 << bit)) != 0);
}

template <typename T>
inline bool bit_is_clear(T &reg, size_t bit)
{
    return ((reg & (1 << bit)) == 0);
}

}
