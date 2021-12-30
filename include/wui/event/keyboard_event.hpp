//
// Copyright (c) 2021-2022 Anton Golovkov (udattsk at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/ud84/WUI
//

#pragma once

#include <cstdint>

namespace wui
{

enum class keyboard_event_type
{
    down, 
    up,
    key
};

static const wchar_t vk_tab = 0x09;
static const wchar_t vk_return = 0x0D;
static const wchar_t vk_back = 0x08;
static const wchar_t vk_del = 0x2E;
static const wchar_t vk_end = 0x23;
static const wchar_t vk_home = 0x24;
static const wchar_t vk_left = 0x25;
static const wchar_t vk_right = 0x27;
static const wchar_t vk_shift = 0x10;

struct keyboard_event
{
    keyboard_event_type type;

    wchar_t key;
};

}
