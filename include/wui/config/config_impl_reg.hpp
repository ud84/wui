//
// Copyright (c) 2021-2022 Anton Golovkov (udattsk at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/ud84/wui
//

#pragma once

#ifdef _WIN32

#include <wui/config/i_config.hpp>

#include <map>

namespace wui
{

namespace config
{

class config_impl_reg : public i_config
{
public:
    config_impl_reg(const std::string &configFile = BASE_APP_KEY, HKEY root = HKEY_CURRENT_USER);

    int32_t get_int(const std::string &section, const std::string &entry, int32_t default_);
    void set_int(const std::string &section, const std::string &entry, int32_t value);

    int64_t get_int64(const std::string &section, const std::string &entry, int64_t default_);
    void set_int64(const std::string &section, const std::string &entry, int64_t value);

    std::string get_string(const std::string &section, const std::string &entry, const std::string &default_);
    void set_string(const std::string &section, const std::string &entry, const std::string value);

    void delete_value(const std::string &section, const std::string &entry);

    void delete_key(const std::string &section);

    virtual bool is_ok() const;

private:

    bool ok;
};

}

}

#endif
