//
// Copyright (c) 2021-2023 Anton Golovkov (udattsk at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/ud84/wui
//

#pragma once

#include <string>

namespace wui
{

enum class error_type
{
    ok,

    file_not_found,
    invalid_json
};

std::string str(error_type);

struct error
{
    error_type type;
    std::string component, message;

    inline bool operator==(const error &lv)
    {
        return type == lv.type && component == lv.component;
    }

    inline bool is_ok() const
    {
        return type == error_type::ok;
    }
};

}
