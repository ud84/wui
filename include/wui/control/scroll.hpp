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
#include <wui/common/color.hpp>
#include <wui/common/orientation.hpp>

#include <functional>
#include <memory>
#include <thread>
#include <atomic>

namespace wui
{

enum class scroll_state
{
    activated,
    relaxed,

    up_end,
    down_end,

    moving
};

enum class scroll_view
{
    none,
    tiny,
    full
};

class scroll : public i_control, public std::enable_shared_from_this<scroll>
{
public:
    scroll(int32_t area, int32_t scroll_pos,
        orientation orientation_ = orientation::vertical,
        std::function<void(scroll_state, int32_t)> callback = nullptr,
        std::string_view theme_control_name = tc, std::shared_ptr<i_theme> theme_ = nullptr);
    virtual ~scroll();

    virtual void draw(graphic &gr, const rect& paint_rect [[maybe_unused]] ) override;

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
    /// Scroll's interface
    void set_area(const int32_t area);
    [[nodiscard]] static int32_t get_buttons_size() noexcept; //! full size buttons [TODO: изменить, когда будет масштабирование]
    void set_scroll_pos(int32_t scroll_pos);
    [[nodiscard]] int32_t get_scroll_pos() const noexcept;

    /// Good to call from mouse whell event
    bool scroll_up();
    bool scroll_down();

    /// If you need to embed a scroll bar in your control, you can draw it with scroll::draw() and subscribe it to events
    void receive_control_events(const event& ev);
    void receive_plain_events(const event& ev);

    [[nodiscard]] scroll_view get_scroll_view() const noexcept;

    void redraw();

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

    std::atomic<double> area, scroll_pos, prev_scroll_pos;
    std::atomic<double> scroll_interval;

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
    std::atomic<bool> worker_started;
    std::atomic<bool> worker_done;

    std::atomic<int32_t> progress;

    scroll_view scroll_view_;

    bool slider_scrolling;
    int32_t slider_click_pos;

    int32_t title_height;

    static constexpr const int32_t tiny_scrollbar_size = 3;
    static constexpr const int32_t full_scrollbar_size = 14;

    void draw_arrow_up(graphic& gr, const rect& button_pos);
    void draw_arrow_left(graphic& gr, const rect& button_pos);
    void draw_arrow_down(graphic& gr, const rect& button_pos);
    void draw_arrow_right(graphic& gr, const rect& button_pos);

    void calc_scroll_interval();
    void calc_vert_scrollbar_params(rect* bar_rect = nullptr, rect* up_button_rect = nullptr, rect* down_button_rect = nullptr, rect* slider_rect = nullptr);
    void calc_hor_scrollbar_params(rect* bar_rect = nullptr, rect* up_button_rect = nullptr, rect* down_button_rect = nullptr, rect* slider_rect = nullptr);
    void calc_scrollbar_params(rect* bar_rect = nullptr, rect* up_button_rect = nullptr, rect* down_button_rect = nullptr, rect* slider_rect = nullptr);

    void move_slider(const int32_t v);

    void start_work(const worker_action action);
    void work();
    void end_work();
};

}
