#pragma once
#include <wui/framework/i_framework.hpp>
namespace wui::framework
{
void init_macos();
class framework_mac_impl : public i_framework
{
public:
    void run() override;
    void stop() override;
    bool started() const override;
    error get_error() const override;
private:
    bool started_ = false;
    error err_;
};
}
