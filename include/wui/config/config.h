//
// config.h - Contains the configuration wrappers interface
//
// Copyright (c) 2014-2023 Anton Golovkov (udattsk at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/ud84/wui
//

#pragma once

#include <string>

#ifdef _WIN32
#include <atlbase.h>
#include <atltrace.h>
#else
extern const std::string BASE_APP_KEY;
using HKEY = int;
static constexpr int HKEY_CURRENT_USER = 0;
#endif

namespace wui
{

namespace config
{

int32_t get_int(const std::string &section, const std::string &entry, int32_t default_, const std::string &configFile = BASE_APP_KEY, HKEY root = HKEY_CURRENT_USER);
void set_int(const std::string &section, const std::string &entry, int32_t value, const std::string &configFile = BASE_APP_KEY, HKEY root = HKEY_CURRENT_USER);

int64_t get_int64(const std::string &section, const std::string &entry, int64_t default_, const std::string &configFile = BASE_APP_KEY, HKEY root = HKEY_CURRENT_USER);
void set_int64(const std::string &section, const std::string &entry, int64_t value, const std::string &configFile = BASE_APP_KEY, HKEY root = HKEY_CURRENT_USER);

std::string get_string(const std::string &section, const std::string &entry, const std::string &default_, const std::string &configFile = BASE_APP_KEY, HKEY root = HKEY_CURRENT_USER);
void set_string(const std::string &section, const std::string &entry, const std::string value, const std::string &configFile = BASE_APP_KEY, HKEY root = HKEY_CURRENT_USER);

void delete_value(const std::string &section, const std::string &entry, const std::string &configFile = BASE_APP_KEY, HKEY root = HKEY_CURRENT_USER);

void delete_key(const std::string &section, const std::string &configFile = BASE_APP_KEY, HKEY root = HKEY_CURRENT_USER);

}

}
