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
#include <wui/system/timer.hpp>

#include <string>
#include <functional>
#include <memory>

namespace wui
{

enum class input_view
{
    singleline,
    multiline
};

class input : public i_control, public std::enable_shared_from_this<input>
{
public:
    input(const std::string &text = "", input_view input_view_ = input_view::singleline, std::shared_ptr<i_theme> theme_ = nullptr);
    ~input();

    virtual void draw(graphic &gr, const rect &);

    virtual void set_position(const rect &position);
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

    void set_text(const std::string &text);
    std::string text() const;

    void set_input_view(input_view input_view_);

    void set_change_callback(std::function<void(const std::string&)> change_callback);

public:
    /// Control name in theme
    static constexpr const char *tc = "input";

    /// Used theme values
    static constexpr const char *tv_background = "background";
    static constexpr const char *tv_text = "text";
    static constexpr const char *tv_selection = "selection";
    static constexpr const char *tv_cursor = "cursor";
    static constexpr const char *tv_border = "border";
    static constexpr const char *tv_border_width = "border_width";
    static constexpr const char *tv_focused_border = "focused_border";
    static constexpr const char *tv_round = "round";
    static constexpr const char *tv_font = "font";

private:
    input_view input_view_;
    std::string text_;
    std::function<void(const std::string&)> change_callback;
    std::shared_ptr<i_theme> theme_;

    rect position_;
    size_t cursor_position, select_start_position, select_end_position;
    
    std::weak_ptr<window> parent;
    int32_t my_subscriber_id;

    timer timer_;

    bool showed_, enabled_;
    bool focused_;
    bool focusing_;
    bool cursor_visible;
    bool selecting;

    int32_t left_shift;

    void receive_event(const event &ev);

    void redraw();

    void redraw_cursor();

    void update_select_positions(bool shift_pressed, size_t start_position, size_t end_position);

    bool clear_selected_text(); /// returns true if selection is not empty

    void select_current_word(int32_t x);

    void select_all();

    bool check_count_valid(size_t count);

    void move_cursor_left();
    void move_cursor_right();

    size_t calculate_mouse_cursor_position(int32_t x);

    void buffer_copy();
    void buffer_cut();
    void buffer_paste();
};

}
