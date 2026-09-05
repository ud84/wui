// Copyright (c) 2026 Intent Garden Org. Boost Software License 1.0.
#pragma once
#include <wui/framework/i_framework.hpp>
namespace wui::framework
{
class framework_wasm_impl : public i_framework
{
public:
    void run() override;
    void stop() override;
    bool started() const override { return running_; }
    error get_error() const override { return {}; }
private:
    bool running_ = false;
};
}
