//
// Copyright (c) 2023 Anton Golovkov (udattsk at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/ud84/wui
//

#pragma once

#include <wui/common/error.hpp>

#include <cstdint>
#include <string>
#include <vector>

namespace wui
{

class i_config
{
public:
    virtual int32_t get_int(const std::string &section, const std::string &entry, int32_t default_) = 0;
    virtual void set_int(const std::string &section, const std::string &entry, int32_t value) = 0;

    virtual int64_t get_int64(const std::string &section, const std::string &entry, int64_t default_) = 0;
    virtual void set_int64(const std::string &section, const std::string &entry, int64_t value) = 0;

    virtual std::string get_string(const std::string &section, const std::string &entry, const std::string &default_) = 0;
    virtual void set_string(const std::string &section, const std::string &entry, const std::string value) = 0;

    virtual void delete_value(const std::string &section, const std::string &entry) = 0;

    virtual void delete_key(const std::string &section) = 0;
       
    virtual error get_error() const = 0;

    virtual ~i_config() {}
};

}
