//
// Copyright (c) 2021-2022 Anton Golovkov (udattsk at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/ud84/WUI
//

#pragma once

#define set_bit(reg, bit)         reg |= (1<<bit)            
#define clear_bit(reg, bit)       reg &= (~(1<<bit))
#define inv_bit(reg, bit)         reg ^= (1<<bit)
#define bit_is_set(reg, bit)       ((reg & (1<<bit)) != 0)
#define bit_is_clear(reg, bit)     ((reg & (1<<bit)) == 0)
