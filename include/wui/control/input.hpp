//
// Copyright (c) 2021-2022 Anton Golovkov (udattsk at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/ud84/WUI
//

#pragma once

#include <wui/control/i_control.hpp>
#include <wui/graphic/graphic.hpp>
#include <wui/event/event.hpp>
#include <wui/common/rect.hpp>
#include <wui/common/color.hpp>
#include <wui/common/timer.hpp>

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
    input(const std::wstring &text = L"", input_view input_view_ = input_view::singleline, std::shared_ptr<i_theme> theme_ = nullptr);
    ~input();

    virtual void draw(graphic &gr);

    virtual void receive_event(const event &ev);

    virtual void set_position(const rect &position);
    virtual rect position() const;

    virtual void set_parent(std::shared_ptr<window> window_);
    virtual void clear_parent();

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

    void set_text(const std::wstring &text);
    std::wstring text() const;

    void set_input_view(input_view input_view_);

    void set_change_callback(std::function<void(const std::wstring&)> change_callback);

private:
    input_view input_view_;
    std::wstring text_, showed_text;
    std::function<void(const std::wstring&)> change_callback;
    std::shared_ptr<i_theme> theme_;

    rect position_;
    size_t cursor_position, select_start_position, select_end_position;
    
    std::weak_ptr<window> parent;

    timer timer_;

    bool showed_, enabled_;
    bool focused_;
    bool focusing_;
    bool cursor_visible;
    bool selecting;

#ifdef _WIN32
    HBRUSH background_brush, selection_brush;
    HPEN cursor_pen, background_pen, border_pen, focused_border_pen;

    HFONT font;

    int32_t left_shift;

    void make_primitives();
    void destroy_primitives();
#endif

    void redraw();

    void redraw_cursor();

    void update_select_positions(bool shift_pressed, size_t start_position, size_t end_position);

    bool clear_selected_text(); /// returns true if selection is not empty

    size_t calculate_mouse_cursor_position(int32_t x);
};

}
