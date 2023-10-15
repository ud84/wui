// Copyright (c) 2021-2023 Anton Golovkov (udattsk at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/ud84/wui
//

#include <wui/common/error.hpp>
#include <map>

namespace wui
{

std::string str(error_type t)
{
    static const std::map<error_type, std::string> names = {
        { error_type::ok,             "ok"             },
        { error_type::file_not_found, "file_not_found" },
        { error_type::invalid_json,   "invalid_json"   }
    };

    auto n = names.find(t);
    if (n != names.end())
    {
        return n->second;
    }

    return "";
}

}
