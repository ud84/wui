//
// Copyright (c) 2023 Anton Golovkov (udattsk at gmail dot com)
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
#include <wui/common/orientation.hpp>

#include <functional>
#include <memory>
#include <thread>

namespace wui
{

enum class scroll_state
{
    up_end,
    down_end,
    moving
};

class scroll : public i_control, public std::enable_shared_from_this<scroll>
{
public:
    scroll(int32_t area, int32_t scroll_pos, double scroll_interval = 1.0,
        orientation orientation_ = orientation::vertical,
        std::function<void(scroll_state, int32_t)> callback = nullptr,
        std::string_view theme_control_name = tc, std::shared_ptr<i_theme> theme_ = nullptr);
    ~scroll();

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
    /// Scroll's interface
    void set_values(int32_t area, double scroll_interval);
    void set_scroll_pos(int32_t scroll_pos);
    int32_t get_scroll_pos() const;

public:
    /// Control name in theme
    static constexpr const char *tc = "scroll";

    /// Used theme values
    static constexpr const char *tv_background = "background";
    static constexpr const char *tv_slider = "slider";
    static constexpr const char *tv_slider_acive = "slider_active";

private:
    std::string tcn; // control name
    std::shared_ptr<i_theme> theme_;

    rect position_;

    std::weak_ptr<window> parent_;
    std::string my_control_sid, my_plain_sid;

    bool showed_, enabled_, topmost_;

    int32_t area, scroll_pos;
    double scroll_interval;

    orientation orientation_;

    std::function<void(scroll_state, int32_t)> callback;

    enum class worker_action
    {
        undefined = 0,

        scroll_up,
        scroll_down,

        scrollbar_show
    };

    worker_action worker_action_;
    std::thread worker;
    bool worker_runned;

    int32_t progress;

    enum class scrollbar_state
    {
        hide,
        tiny,
        full
    };
    scrollbar_state scrollbar_state_;

    bool slider_scrolling;
    int32_t slider_click_pos;
    int32_t prev_scroll_pos;

    int32_t title_height;

    static const int32_t tiny_scrollbar_size = 3;
    static const int32_t full_scrollbar_size = 14;

    void receive_control_events(const event& ev);
    void receive_plain_events(const event& ev);

    void redraw();

    void draw_arrow_up(graphic& gr, rect button_pos);
    void draw_arrow_left(graphic& gr, rect button_pos);
    void draw_arrow_down(graphic& gr, rect button_pos);
    void draw_arrow_right(graphic& gr, rect button_pos);
        
    void calc_vert_scrollbar_params(rect* bar_rect = nullptr, rect* up_button_rect = nullptr, rect* down_button_rect = nullptr, rect* slider_rect = nullptr);
    void calc_hor_scrollbar_params(rect* bar_rect = nullptr, rect* up_button_rect = nullptr, rect* down_button_rect = nullptr, rect* slider_rect = nullptr);
    void calc_scrollbar_params(rect* bar_rect = nullptr, rect* up_button_rect = nullptr, rect* down_button_rect = nullptr, rect* slider_rect = nullptr);

    void move_slider(int32_t y);

    void scroll_up();
    void scroll_down();

    void start_work(worker_action action);
    void work();
    void end_work();
};

}
