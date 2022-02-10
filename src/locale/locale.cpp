//
// Copyright (c) 2021-2022 Anton Golovkov (udattsk at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/ud84/wui
//

#include <wui/locale/locale.hpp>
#include <wui/locale/locale_impl.hpp>

namespace wui
{

static std::shared_ptr<i_locale> instance = nullptr;
static std::string dummy_string;
static std::vector<uint8_t> dummy_image;

/// Interface

void set_locale_from_json(const std::string &name, const std::string &json)
{
    instance.reset();
    instance = std::shared_ptr<i_locale>(new locale_impl(name));
    instance->load_json(json);
}

void set_locale_from_file(const std::string &name, const std::string &file_name)
{
    instance.reset();
    instance = std::shared_ptr<i_locale>(new locale_impl(name));
    instance->load_file(file_name);
}

void set_locale_empty(const std::string &name)
{
    instance.reset();
    instance = std::shared_ptr<i_locale>(new locale_impl(name));
}

std::shared_ptr<i_locale> get_locale()
{
    return instance;    
}

void set_locale_value(const std::string &section, const std::string &value, const std::string &str)
{
    if (instance)
    {
        instance->set(section, value, str);
    }
}

const std::string &locale(const std::string &section, const std::string &value)
{
    if (instance)
    {
        return instance->get(section, value);
    }
    return dummy_string;
}

}
