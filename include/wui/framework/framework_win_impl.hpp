//
// Copyright (c) 2023 Anton Golovkov (udattsk at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/ud84/wui
//

#pragma once

#include <wui/framework/i_framework.hpp>

#include <cstdint>
#include <string>
#include <vector>

namespace wui
{

namespace framework
{

class framework_win_impl : public i_framework
{
public:
    framework_win_impl();

    virtual void run();
    virtual void stop();

    virtual bool runned() const;

    virtual error get_error() const;

private:
    bool runned_;

    error err;
};

}


}

