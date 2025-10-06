//
// Copyright (c) 2021-2023 Anton Golovkov (udattsk at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://gitverse.ru/udattsk/wui
//

#pragma once

#include <string>
#include <wui/system/system_context.hpp>

namespace wui
{

/// Clipboard's functions
void clipboard_put(std::string_view text, system_context &context);

bool is_text_in_clipboard(system_context &context);
std::string clipboard_get_text(system_context &context);

}
