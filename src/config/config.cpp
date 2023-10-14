//
// Copyright (c) 2023 Anton Golovkov (udattsk at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/ud84/wui
//

#include <wui/config/config.hpp>
#include <wui/config/config_impl_ini.hpp>
#include <wui/config/config_impl_reg.hpp>

namespace wui
{

namespace config
{

static std::shared_ptr<i_config> instance = nullptr;

/// Interface

bool use_ini_file(const std::string &file_name)
{
    instance.reset();
    instance = std::shared_ptr<i_config>(new config_impl_ini(file_name));

    return instance->is_ok();
}
#ifdef _WIN32
bool use_registry(const std::string &app_key, HKEY root)
{
    instance.reset();
    instance = std::shared_ptr<i_config>(new config_impl_reg(app_key, root));

    return instance->is_ok();
}
#endif

std::string get_error()
{
    if (instance)
    {
        return instance->get_error();
    }
    return "";
}

int32_t get_int(const std::string &section, const std::string &entry, int32_t default_)
{
    if (instance)
    {
        return instance->get_int(section, entry, default_);
    }
    return -1;
}

void set_int(const std::string &section, const std::string &entry, int32_t value)
{
    if (instance)
    {
        instance->set_int(section, entry, value);
    }
}

int64_t get_int64(const std::string &section, const std::string &entry, int64_t default_)
{
    if (instance)
    {
        return instance->get_int64(section, entry, default_);
    }
    return -1;
}

void set_int64(const std::string &section, const std::string &entry, int64_t value)
{
    if (instance)
    {
        instance->set_int64(section, entry, value);
    }
}

std::string get_string(const std::string &section, const std::string &entry, const std::string &default_)
{
    if (instance)
    {
        return instance->get_string(section, entry, default_);
    }
    return "";
}

void set_string(const std::string &section, const std::string &entry, const std::string value)
{
    if (instance)
    {
        instance->set_string(section, entry, value);
    }
}

void delete_value(const std::string &section, const std::string &entry)
{
    if (instance)
    {
        instance->delete_value(section, entry);
    }
}

void delete_key(const std::string &section)
{
    if (instance)
    {
        instance->delete_key(section);
    }
}

}

}
