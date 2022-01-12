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

#ifdef _WIN32
static const wchar_t vk_tab = 0x09;
static const wchar_t vk_return = 0x0D;

static const wchar_t vk_back = 0x08;
static const wchar_t vk_del = 0x2E;

static const wchar_t vk_end = 0x23;
static const wchar_t vk_home = 0x24;

static const wchar_t vk_up = 0x26;
static const wchar_t vk_down = 0x28;
static const wchar_t vk_left = 0x25;
static const wchar_t vk_right = 0x27;

/// The modifier keys
static const wchar_t vk_capital = 0x14;
static const wchar_t vk_shift = 0x10;
static const wchar_t vk_alt = 0x12;
static const wchar_t vk_lcontrol = 0xA2;
static const wchar_t vk_rcontrol = 0xA3;
static const wchar_t vk_insert = 0x2D;
#elif __linux__
static const wchar_t vk_tab = 0x17;
static const wchar_t vk_return = 0x24;

static const wchar_t vk_back = 0x16;
static const wchar_t vk_del = 0x77;

static const wchar_t vk_end = 0x73;
static const wchar_t vk_home = 0x6E;

static const wchar_t vk_up = 0x6F;
static const wchar_t vk_down = 0x74;
static const wchar_t vk_left = 0x71;
static const wchar_t vk_right = 0x72;

/// The modifier keys
static const wchar_t vk_capital = 0x02;
static const wchar_t vk_shift = 0x01;
static const wchar_t vk_alt = 0x08;
static const wchar_t vk_lcontrol = 0x04;
static const wchar_t vk_rcontrol = 0x04;
static const wchar_t vk_insert = 0;
#endif

struct keyboard_event
{
    keyboard_event_type type;

    wchar_t modifier; /// vk_capital or vk_shift or vk_alt or vk_insert
    wchar_t key;
};

}
