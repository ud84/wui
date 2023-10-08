//
// Copyright (c) 2021-2022 Anton Golovkov (udattsk at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/ud84/wui
//

#ifdef _WIN32

#include <wui/config/config_impl_reg.hpp>
#include <wui/system/tools.hpp>

#include <boost/nowide/convert.hpp>

namespace wui
{

namespace config
{

config_impl_reg::config_impl_reg(const std::string &base_application_key_, HKEY root_)
	: base_application_key(base_application_key_), root(root_),
	ok(false)
{
}

config_impl_reg::~config_impl_reg()
{
}

int32_t config_impl_reg::get_int(const std::string &section, const std::string &entry, int32_t default_)
{
	return default_;
}

void config_impl_reg::set_int(const std::string &section, const std::string &entry, int32_t value)
{

}

int64_t config_impl_reg::get_int64(const std::string &section, const std::string &entry, int64_t default_)
{
	return default_;
}

void config_impl_reg::set_int64(const std::string &section, const std::string &entry, int64_t value)
{

}

std::string config_impl_reg::get_string(const std::string &section, const std::string &entry, const std::string &default_)
{
	return default_;
}

void config_impl_reg::set_string(const std::string &section, const std::string &entry, const std::string value)
{

}

void config_impl_reg::delete_value(const std::string &section, const std::string &entry)
{

}

void config_impl_reg::delete_key(const std::string &section)
{
    
}

bool config_impl_reg::is_ok() const
{
    return ok;
}

}

}

#endif
