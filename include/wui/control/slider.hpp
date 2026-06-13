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

    virtual ~slider();

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
    /// Slider's interface
    void set_range(int32_t from, int32_t to);
    void set_value(int32_t value);
    int32_t get_value() const;
    void set_centered_mode(bool centered); // Установить режим с 0 посередине

    void set_callback(std::function<void(int32_t)> change_callback);
    void set_drag_end_callback(std::function<void()> drag_end_callback);

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
    static constexpr double _pos_progress_max = 0.95; /// Если прогресс >= 95%, считаем что достигли максимума
    slider_orientation orientation;
    int32_t from, to, value;
    bool centered_mode; // Если true, 0 находится посередине (для диапазонов типа -7..+7)
    std::function<void(int32_t)> change_callback;
    std::function<void()> drag_end_callback;

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
