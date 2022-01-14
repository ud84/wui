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

std::string theme_control_to_str(theme_control tc);
std::string theme_value_to_str(theme_value tv);

theme_control theme_control_from_str(const std::string &str);
theme_value theme_value_from_str(const std::string &str);

static const char * dark_json = "{ \"controls\": [ { \"type\": \"window\", \"background\": \"#131519\", \"text\": \"#f5f5f0\", \"active_button\": \"#3b3d41\", \"caption_font\": { \"name\": \"Segoe UI\", \"size\": 18 } }, { \"type\": \"button\", \"calm\": \"#06a5df\", \"active\": \"#1aafe9\", \"border\": \"##00a0d2\", \"focused_border\": \"#dcd2dc\", \"text\": \"#f0f1f1\", \"disabled\": \"#a5a5a0\", \"round\": 0, \"font\": { \"name\": \"Segoe UI\", \"size\": 18 } }, { \"type\": \"input\", \"background\": \"#27292d\", \"text\": \"#f0ebf0\", \"selection\": \"#264f78\", \"cursor\": \"#d2d2d2\", \"border\": \"#8c8c8c\", \"focused_border\": \"#c8c8c8\", \"round\": 0, \"font\": { \"name\": \"Segoe UI\", \"size\": 16 } }, { \"type\": \"tooltip\", \"background\": \"#b4aabe\", \"border\": \"#f1f2f7\", \"text\": \"#061912\", \"round\": 0, \"text_indent\": 3, \"font\": { \"name\": \"Segoe UI\", \"size\": 18 } } ] } ";
static const char * white_json = "{ \"controls\": [ { \"type\": \"window\", \"background\": \"#f0f0f0\", \"text\": \"#191914\", \"active_button\": \"#dcdcdc\", \"font\": { \"name\": \"Segoe UI\", \"size\": 18 } }, { \"type\": \"button\", \"calm\": \"#06a5df\", \"active\": \"#1aafe9\", \"border\": \"#00a0d2\", \"focused_border\": \"#140a14\", \"text\": \"#181818\", \"disabled\": \"#cdcdc8\", \"round\": 0, \"font\": { \"name\": \"Segoe UI\", \"size\": 18 } }, { \"type\": \"input\", \"background\": \"#dcdcdc\", \"text\": \"#191914\", \"selection\": \"#99c9ef\", \"cursor\": \"#141414\", \"border\": \"#28788c\", \"focused_border\": \"#140a14\", \"round\": 0, \"font\": { \"name\": \"Segoe UI\", \"size\": 18 } }, { \"type\": \"tooltip\", \"background\": \"#f1f2f7\", \"border\": \"#767676\", \"text\": \"#061912\", \"round\": 0, \"text_indent\": 3, \"font\": { \"name\": \"Segoe UI\", \"size\": 18 } } ] } ";

}
