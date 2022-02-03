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
#include <atomic>
#include <thread>

namespace wui
{

class list : public i_control, public std::enable_shared_from_this<list>
{
public:
    list(std::shared_ptr<i_theme> theme_ = nullptr);
    ~list();

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

    /// list interface
    struct column
    {
        int32_t width;
        std::string caption;
    };
    void update_columns(const std::vector<column> &columns);

    enum class list_mode
    {
        simple,
        auto_select
    };
    void set_mode(list_mode mode);
    
    void select_item(int32_t n_item);
    int32_t selected_item() const;

    void set_column_width(int32_t n_item, int32_t width);
    void set_item_height(int32_t height);
    
    void set_item_count(int32_t count);

    enum class item_state
    {
        normal,
        active,
        selected
    };

    void set_draw_callback(std::function<void(graphic&, int32_t, const rect&, item_state state, const std::vector<column> &columns)> draw_callback_);
    void set_item_change_callback(std::function<void(int32_t)> item_change_callback_);
    void set_item_activate_callback(std::function<void(int32_t)> item_activate_callback_);
    void set_column_click_callback(std::function<void(int32_t)> column_click_callback_);
    void set_item_right_click_callback(std::function<void(int32_t)> item_right_click_callback_);
    
public:
    /// Control name in theme
    static constexpr const char *tc = "list";

    /// Used theme values
    static constexpr const char *tv_background = "background";
    static constexpr const char *tv_border = "border";
    static constexpr const char *tv_focused_border = "focused_border";
    static constexpr const char *tv_border_width = "border_width";
    static constexpr const char *tv_text_indent = "text_indent";
    static constexpr const char *tv_title = "title";
    static constexpr const char *tv_title_text = "title_text";
    static constexpr const char *tv_scrollbar = "scrollbar";
    static constexpr const char *tv_scrollbar_slider = "scrollbar_slider";
    static constexpr const char *tv_scrollbar_slider_acive = "scrollbar_slider_active";
    static constexpr const char *tv_selected_item = "selected_item";
    static constexpr const char *tv_active_item = "active_item";
    static constexpr const char *tv_round = "round";
    static constexpr const char *tv_font = "font";

private:
    std::shared_ptr<i_theme> theme_;

    rect position_;

    std::weak_ptr<window> parent;
    std::string my_subscriber_id;

    bool showed_, enabled_, focused_;

    std::vector<column> columns;

    list_mode mode;

    std::atomic<int32_t> item_height, item_count, selected_item_, active_item_, start_item;

    enum class worker_action
    {
        undefined = 0,

        scroll_up,
        scroll_down,
        
        select_up,
        select_down,

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
    int32_t prev_scroll_pos;

    int32_t title_height;

    static const int32_t tiny_scrollbar_width = 3;
    static const int32_t full_scrollbar_width = 14;

    std::function<void(graphic&, int32_t, const rect&, item_state state, const std::vector<column> &columns)> draw_callback;
    std::function<void(int32_t)> item_change_callback;
    std::function<void(int32_t)> item_activate_callback;
    std::function<void(int32_t)> column_click_callback;
    std::function<void(int32_t)> item_right_click_callback;
    
    void receive_event(const event &ev);

    void redraw();

    void redraw_item(int32_t item);

    void draw_titles(graphic &gr_);

    void draw_items(graphic &gr_);

    void draw_scrollbar(graphic &gr_);
    void draw_arrow_up(graphic &gr, rect button_pos);
    void draw_arrow_down(graphic &gr, rect button_pos);

    int32_t get_visible_item_count() const;
    
    void scroll_up();
    void scroll_down();

    void calc_scrollbar_params(rect *bar_rect = nullptr, rect *top_button_rect = nullptr, rect *bottom_button_rect = nullptr, rect *slider_rect = nullptr);
    bool is_click_on_scrollbar(int32_t x);
    void update_selected_item(int32_t y);
    void update_active_item(int32_t y);

    void start_work(worker_action action);
    void work();
    void end_work();
};

}
