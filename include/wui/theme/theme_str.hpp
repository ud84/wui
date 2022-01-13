//
// Copyright (c) 2021-2022 Anton Golovkov (udattsk at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/ud84/wui
//

#pragma once

#include <wui/theme/i_theme.hpp>

#include <string>

namespace wui
{

std::string str_theme_control(theme_control tc);
std::string str_theme_value(theme_value tv);

static const char * dark_json = "{\"button\":{}, \"image\":{}, \"button\":{}, \"input\":{}, \"tooltip\":{}}";
static const char * white_json = "{\"button\":{}, \"image\":{}, \"button\":{}, \"input\":{}, \"tooltip\":{}}";

}
