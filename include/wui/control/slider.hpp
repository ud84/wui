//
// Copyright (c) 2021-2022 Anton Golovkov (udattsk at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/ud84/wui
//

#pragma once

#include <wui/control/i_control.hpp>
#include <wui/graphic/graphic.hpp>
#include <wui/event/event.hpp>
#include <wui/common/rect.hpp>
#include <wui/common/color.hpp>

#include <string>
#include <functional>
#include <memory>

namespace wui
{

enum class slider_orientation
{
    vertical,
    horizontal
};

class slider : public i_control, public std::enable_shared_from_this<slider>
{
public:
    slider(int32_t from, int32_t to, int32_t value, std::function<void(int32_t)> change_callback, slider_orientation orientation = slider_orientation::horizontal, std::shared_ptr<i_theme> theme_ = nullptr);

    ~slider();

    /// i_control impl
    virtual void draw(graphic &gr, const rect &);

    virtual void set_position(const rect &position, bool redraw = true);
    virtual rect position() const;

    virtual void set_parent(std::shared_ptr<window> window_);
    virtual void clear_parent();

    virtual bool topmost() const;

    virtual void set_focus();
    virtual bool remove_focus();
    virtual bool focused() const;
    virtual bool focusing() const;

    virtual void update_theme(std::shared_ptr<i_theme> theme_ = nullptr);

    virtual void show();
    virtual void hide();
    virtual bool showed() const;

    virtual void enable();
    virtual void disable();
    virtual bool enabled() const;

    /// Slider's interface
    void set_range(int32_t from, int32_t to);
    void set_value(int32_t value);

    void set_callback(std::function<void(int32_t)> change_callback);

public:
    /// Control name in theme
    static constexpr const char *tc = "slider";

    /// Used theme values
    static constexpr const char *tv_perform = "perform";
    static constexpr const char *tv_remain = "remain";
    static constexpr const char *tv_active = "active";

private:
    slider_orientation orientation;
    int32_t from, to, value;
    std::function<void(int32_t)> change_callback;
    std::shared_ptr<i_theme> theme_;

    rect position_;

    std::weak_ptr<window> parent;
    std::string my_control_sid, my_plain_sid;

    bool showed_, enabled_;
    bool active, focused_;

    void receive_control_events(const event &ev);
    void receive_plain_events(const event &ev);

    void redraw();
};

}
