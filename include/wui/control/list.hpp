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
#include <vector>
#include <functional>
#include <memory>

namespace wui
{

class list : public i_control, public std::enable_shared_from_this<list>
{
public:
    list(std::shared_ptr<i_theme> theme_ = nullptr);
    ~list();

    virtual void draw(graphic &gr);

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

    /// list interface
    void add_column(int32_t width, const std::string &caption);
    
    void select_item(int32_t n_item);
    int32_t selected_item() const;

    void set_column_width(int32_t n_item, int32_t width);
    void set_item_height(int32_t height);
    
    void set_item_count(int32_t count);

    void set_draw_callback(std::function<void(graphic&, int32_t, const rect&, bool selected)> draw_callback_);
    void set_item_change_callback(std::function<void(int32_t)> item_change_callback_);
    void set_column_click_callback(std::function<void(int32_t)> column_click_callback_);
    void set_item_click_callback(std::function<void(int32_t)> item_click_callback_);
    void set_item_double_click_callback(std::function<void(int32_t)> item_double_click_callback_);
    void set_item_right_click_callback(std::function<void(int32_t)> item_right_click_callback_);
    
public:
    /// Control name in theme
    static constexpr const char *tc = "list";

    /// Used theme values
    static constexpr const char *tv_background = "background";
    static constexpr const char *tv_border = "border";
    static constexpr const char *tv_border_width = "border_width";
    static constexpr const char *tv_title = "title";
    static constexpr const char *tv_title_text = "title_text";
    static constexpr const char *tv_scrollbar = "scrollbar";
    static constexpr const char *tv_scrollbar_slider = "scrollbar_slider";
    static constexpr const char *tv_scrollbar_slider_acive = "scrollbar_slider_active";
    static constexpr const char *tv_round = "round";
    static constexpr const char *tv_font = "font";

private:
    std::shared_ptr<i_theme> theme_;

    rect position_;

    std::weak_ptr<window> parent;

    bool showed_, enabled_, focused_;

    struct column
    {
        int32_t width;
        std::string caption;
    };
    std::vector<column> columns;

    int32_t item_height, item_count, selected_item_, start_item;

    std::function<void(graphic&, int32_t, const rect&, bool selected)> draw_callback;
    std::function<void(int32_t)> item_change_callback;
    std::function<void(int32_t)> column_click_callback;
    std::function<void(int32_t)> item_click_callback;
    std::function<void(int32_t)> item_double_click_callback;
    std::function<void(int32_t)> item_right_click_callback;
    
    void receive_event(const event &ev);

    void redraw();

    void draw_titles(graphic &gr_);

    void draw_items(graphic &gr_);

    void draw_scrollbar(graphic &gr_);

    int32_t get_title_height() const;
    int32_t get_visible_item_count() const;
    
    void scroll_up();
    void scroll_down();
};

}

