//
// Copyright (c) 2021-2022 Anton Golovkov (udattsk at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/ud84/wui

#pragma once

#include <string>

namespace wui
{

// Converting between unicode / ANSI
std::string to_multibyte(const std::wstring &input);

// Converting between ANSI / unicode
std::wstring to_widechar(const std::string &input);

}
