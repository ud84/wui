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
#include <wui/event/event.hpp>
#include <wui/common/rect.hpp>

// TODO: enum class cursor {}, вынести определение в отдельный файл enum class cursor
#include <wui/system/tools.hpp>

#include <functional>
#include <memory>

namespace wui
{

enum class splitter_orientation
{
    vertical,
    horizontal
};

class splitter : public i_control, public std::enable_shared_from_this<splitter>
{
public:
    splitter(splitter_orientation orientation, std::function<void(int32_t, int32_t)> callback, std::string_view theme_control_name = tc, std::shared_ptr<i_theme> theme_ = nullptr);

    virtual ~splitter();

    /// i_control impl
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
    /// Splitter interface
    void set_callback(std::function<void(int32_t, int32_t)> callback_);

    void set_margins(int32_t min_, int32_t max_);

    void redraw();

public:
    /// Control name in theme
    static constexpr const char *tc = "splitter";

    /// Used theme values
    static constexpr const char *tv_calm = "calm";
    static constexpr const char *tv_active = "active";

private:
    splitter_orientation orientation;
    std::function<void(int32_t, int32_t)> callback;
    int32_t margin_min, margin_max;

    std::string tcn; /// control name in theme
    std::shared_ptr<i_theme> theme_;

    rect position_;

    std::weak_ptr<window> parent_;
    std::string my_control_sid, my_plain_sid;

    bool showed_, enabled_, active, topmost_;
    cursor cursor_{ cursor::no_ };
    rect prev_pos;

    void receive_control_events(const event &ev);
    void receive_plain_events(const event &ev);

};

}
