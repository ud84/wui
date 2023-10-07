//
// config.hpp - Contains the configuration wrappers interface
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

#include <memory>

namespace wui
{

namespace config
{

bool use_ini_file(const std::string &file_name);
#ifdef _WIN32
bool use_registry(const std::string &app_key = BASE_APP_KEY, HKEY root = HKEY_CURRENT_USER);
#endif

int32_t get_int(const std::string &section, const std::string &entry, int32_t default_);
void set_int(const std::string &section, const std::string &entry, int32_t value);

int64_t get_int64(const std::string &section, const std::string &entry, int64_t default_);
void set_int64(const std::string &section, const std::string &entry, int64_t value);

std::string get_string(const std::string &section, const std::string &entry, const std::string &default_);
void set_string(const std::string &section, const std::string &entry, const std::string value);

void delete_value(const std::string &section, const std::string &entry);

void delete_key(const std::string &section);

}

}