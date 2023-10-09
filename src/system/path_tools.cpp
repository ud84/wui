//
// Copyright (c) 2021-2022 Anton Golovkov (udattsk at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/ud84/wui
//

#include <wui/system/path_tools.hpp>

#ifndef _WIN32

#include <pwd.h>

#include <stdlib.h>

#endif

namespace wui
{

std::string real_path(const std::string &relative_path)
{
#ifdef _WIN32
    return relative_path;
#else
    auto index = relative_path.find("~/");
    if (index != std::string::npos)
    {
        const char *homedir = getenv("HOME");
        if (homedir != NULL)
        {
            homedir = getpwuid(getuid())->pw_dir;

            std::string new_path = relative_path;

            new_path.replace(index, 1, homedir);

            return new_path;
        }
    }

    return relative_path;
#endif
}

}
