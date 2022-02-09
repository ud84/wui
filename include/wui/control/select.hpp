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

#include <wui/control/list.hpp>

#include <string>
#include <vector>
#include <functional>
#include <memory>

namespace wui
{

class image;

struct select_item;

typedef std::vector<select_item> select_items_t;

struct select_item
{
    int32_t id;

    std::string text;
    std::shared_ptr<image> image_;

    inline bool operator==(const int32_t id_)
    {
        return id == id_;
    }
};

class select : public i_control, public std::enable_shared_from_this<select>
{
public:
    select(std::shared_ptr<i_theme> theme_ = nullptr);
    ~select();

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

    void set_items(const select_items_t &items);
    void update_item(const select_item &mi);
    void swap_items(int32_t first_item_id, int32_t second_item_id);
    void delete_item(int32_t id);

    void set_item_height(int32_t item_height);

public:
    /// Control name in theme
    static constexpr const char *tc = "select";

    /// Used theme values
    static constexpr const char *tv_background = "background";
    static constexpr const char *tv_border = "border";
    static constexpr const char *tv_border_width = "border_width";
    static constexpr const char *tv_text = "text";
    static constexpr const char *tv_selected_item = "selected_item";
    static constexpr const char *tv_scrollbar = "scrollbar";
    static constexpr const char *tv_scrollbar_slider = "scrollbar_slider";
    static constexpr const char *tv_scrollbar_slider_acive = "scrollbar_slider_active";
    static constexpr const char *tv_round = "round";
    static constexpr const char *tv_font = "font";

private:
    std::shared_ptr<i_theme> list_theme;
    std::shared_ptr<list> list_;

    std::shared_ptr<i_theme> theme_;

    rect position_;

    std::weak_ptr<window> parent;
    std::string my_subscriber_id;

    std::shared_ptr<i_control> activation_control;

    std::vector<select_item> items;

    int32_t max_text_width;

    bool showed_;
    bool size_updated;

    void update_list_theme();

    void receive_event(const event &ev);

    void update_size();

    void draw_list_item(wui::graphic &gr, int32_t n_item, const wui::rect &item_rect_, list::item_state state, const std::vector<list::column> &columns);
    void activate_list_item(int32_t n_item);
};

}
