//
// Copyright (c) 2021-2026 Intent Garden Org
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
//

#pragma once

#include <wui/control/i_control.hpp>
#include <wui/graphic/graphic.hpp>
#include <wui/common/rect.hpp>
//#include <wui/common/color.hpp>
#include <wui/common/orientation.hpp>
#include <wui/event/event.hpp>

#include <string>
#include <functional>
#include <memory>

namespace wui
{

class progress : public i_control, public std::enable_shared_from_this<progress>
{
public:
    progress(int32_t from, int32_t to, int32_t value, orientation orientation_ = orientation::horizontal, std::string_view theme_control_name = tc, std::shared_ptr<i_theme> theme_ = nullptr);
    virtual ~progress();

    virtual void draw(graphic &gr, const rect&) override;

    virtual void set_position(const rect& position) override;
    [[nodiscard]] virtual rect position() const override;

    virtual void set_parent(std::shared_ptr<window> window_) override;
    [[nodiscard]] virtual std::weak_ptr<window> parent() const override;
    virtual void clear_parent() override;

    virtual void set_topmost(bool yes) override;
    [[nodiscard]] virtual bool topmost() const override;

    virtual void update_theme_control_name(std::string_view theme_control_name) override;
    virtual void update_theme(std::shared_ptr<i_theme> theme_ = nullptr) override;

    virtual void show() override;
    virtual void hide() override;
    [[nodiscard]] virtual bool showed() const override;

    virtual void enable() override;
    virtual void disable() override;
    [[nodiscard]] virtual bool enabled() const override;

    [[nodiscard]] virtual bool focused() const override;
    [[nodiscard]] virtual bool focusing() const override;

    [[nodiscard]] virtual error get_error() const override;

public:
    /// Progress's interface
    void set_range(int32_t from, int32_t to);
    void set_value(int32_t value);
    void set_click_callback(std::function<void(int32_t, bool)> click_callback);
    void set_point(int32_t value, bool is_max);

    void redraw();

public:
    /// Control name in theme
    static constexpr const char *tc = "progress"; /// control name in theme

    /// Used theme values
    static constexpr const char *tv_border = "border";
    static constexpr const char *tv_border_width = "border_width";
    static constexpr const char *tv_background = "background";
    static constexpr const char *tv_meter = "meter";
    static constexpr const char* tv_round = "round";
    static constexpr const char* tv_meter_positive = "meter_positive";
    static constexpr const char* tv_meter_negative = "meter_negative";

private:
    std::string tcn;
    std::shared_ptr<i_theme> theme_;

    rect position_;

    std::weak_ptr<window> parent_;
    std::string my_control_sid;

    bool showed_, topmost_;

    int32_t from, to, value;

    orientation orientation_;

    std::function<void(int32_t, bool)> click_callback;
    bool has_min_point{ false };
    bool has_max_point{ false };
    int32_t min_point{ 0 };
    int32_t max_point{ 0 };

    void receive_control_events(const event &ev);
};

}
