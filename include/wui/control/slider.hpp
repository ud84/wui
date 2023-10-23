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
    slider(int32_t from,
        int32_t to,
        int32_t value,
        std::function<void(int32_t)> change_callback,
        slider_orientation orientation = slider_orientation::horizontal,
        std::string_view theme_control_name = tc, std::shared_ptr<i_theme> theme_ = nullptr);

    ~slider();

    /// i_control impl
    virtual void draw(graphic &gr, const rect &);

    virtual void set_position(const rect &position, bool redraw = true);
    virtual rect position() const;

    virtual void set_parent(std::shared_ptr<window> window_);
    virtual std::weak_ptr<window> parent() const;
    virtual void clear_parent();

    virtual void set_topmost(bool yes);
    virtual bool topmost() const;

    virtual void update_theme_control_name(std::string_view theme_control_name);
    virtual void update_theme(std::shared_ptr<i_theme> theme_ = nullptr);

    virtual void show();
    virtual void hide();
    virtual bool showed() const;

    virtual void enable();
    virtual void disable();
    virtual bool enabled() const;

    virtual bool focused() const;
    virtual bool focusing() const;

    virtual error get_error() const;

public:
    /// Slider's interface
    void set_range(int32_t from, int32_t to);
    void set_value(int32_t value);
    int32_t get_value() const;

    void set_callback(std::function<void(int32_t)> change_callback);

public:
    /// Control name in theme
    static constexpr const char *tc = "slider";

    /// Used theme values
    static constexpr const char *tv_perform = "perform";
    static constexpr const char *tv_remain = "remain";
    static constexpr const char *tv_active = "active";
    static constexpr const char *tv_slider_width = "slider_width";
    static constexpr const char *tv_slider_height = "slider_height";
    static constexpr const char *tv_slider_round = "slider_round";

private:
    slider_orientation orientation;
    int32_t from, to, value;
    std::function<void(int32_t)> change_callback;

    std::string tcn; /// control name in theme
    std::shared_ptr<i_theme> theme_;

    rect position_;

    std::weak_ptr<window> parent_;
    std::string my_control_sid, my_plain_sid;

    bool showed_, enabled_, topmost_;
    bool active, focused_;

    bool slider_scrolling, mouse_on_control;

    rect slider_position;

    double diff_size;

    void receive_control_events(const event &ev);
    void receive_plain_events(const event &ev);

    void redraw(bool clear = false);

    void calc_consts();

    void move_slider(int32_t x, int32_t y);
    void scroll_up();
    void scroll_down();
};

}
