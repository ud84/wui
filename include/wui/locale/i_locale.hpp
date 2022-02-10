//
// Copyright (c) 2021-2022 Anton Golovkov (udattsk at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/ud84/wui
//

#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace wui
{

class i_locale
{
public:
    virtual std::string get_name() const = 0;

    virtual void set(const std::string &section, const std::string &value, const std::string &str) = 0;
    virtual const std::string &get(const std::string &section, const std::string &value) const = 0;

    virtual void load_json(const std::string &json) = 0;
    virtual void load_file(const std::string &file_name) = 0;
    virtual void load_locale(const i_locale &locale_) = 0;

    virtual ~i_locale() {}
};

}
