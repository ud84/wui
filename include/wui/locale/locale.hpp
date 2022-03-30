//
// Copyright (c) 2021-2022 Anton Golovkov (udattsk at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/ud84/wui
//

#pragma once

#include <wui/locale/i_locale.hpp>

#include <memory>

namespace wui
{

/// Set and get the current locale
#ifdef _WIN32
bool set_locale_from_resource(const std::string &name, int32_t resource_index, const std::string &resource_section);
#endif
bool set_locale_from_json(const std::string &name, const std::string &json);
bool set_locale_from_file(const std::string &name, const std::string &file_name);
void set_locale_empty(const std::string &name);

/// Return the pointer to current default locale instance
std::shared_ptr<i_locale> get_locale();

/// Set locale section value
void set_locale_value(const std::string &section, const std::string &value, const std::string &str);

/// Return the item's string value by current locale
const std::string &locale(const std::string &section, const std::string &value);

}
