//
// Copyright (c) 2021-2026 Intent Garden Org
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
//

#pragma once

#include <wui/framework/i_framework.hpp>

#include <cstdint>
#include <string>
#include <vector>
#include <atomic>
#include <condition_variable>

namespace wui
{

namespace framework
{

class framework_lin_impl : public i_framework
{
public:
    framework_lin_impl();
    virtual ~framework_lin_impl() override = default;

    virtual void run() override;
    virtual void stop() override;

    [[nodiscard]] virtual bool started() const override;

    [[nodiscard]] virtual error get_error() const override;

private:
    std::atomic<bool> started_;
    std::mutex mtx_cv_;
    std::condition_variable cv_;

    error err;
};

}


}

