//
// Copyright (c) 2021-2026 Intent Garden Org
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
//

#include <wui/system/path_tools.hpp>

#if !defined(_WIN32) && !defined(__EMSCRIPTEN__)

#include <pwd.h>

#include <stdlib.h>

#include <unistd.h>

#endif

namespace wui
{
#ifdef __APPLE__
std::string macos_resource_path(std::string_view path);
#endif

std::string real_path(std::string_view relative_path)
{
#if !defined(_WIN32) && !defined(__EMSCRIPTEN__)
    auto index = relative_path.find("~/");
    if (index != std::string::npos)
    {
        const char *homedir = getenv("HOME");
        if (homedir != NULL)
        {
            homedir = getpwuid(getuid())->pw_dir;

            std::string new_path(relative_path.begin(), relative_path.end());

            new_path.replace(index, 1, homedir);

            return new_path;
        }
    }
#endif

#ifdef __APPLE__
    return macos_resource_path(relative_path);
#else
    return std::string(relative_path);
#endif
}

}
