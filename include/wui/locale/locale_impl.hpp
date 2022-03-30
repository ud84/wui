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

#include <map>

namespace wui
{

class locale_impl : public i_locale
{
public:
    locale_impl(const std::string &name);

    virtual std::string get_name() const;

    virtual void set(const std::string &section, const std::string &value, const std::string &str);
    virtual const std::string &get(const std::string &section, const std::string &value) const;

#ifdef _WIN32
    virtual void load_resource(int32_t resource_index, const std::string &resource_section);
#endif
    virtual void load_json(const std::string &json);
    virtual void load_file(const std::string &file_name);
    virtual void load_locale(const i_locale &locale_);

    virtual bool is_ok() const;

private:
    std::string name;

    std::map<std::string, std::string> strings;

    std::string dummy_string;

    bool ok;
};

}
