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

#include <wui/control/image.hpp>

#include <string>
#include <vector>
#include <functional>
#include <memory>

namespace wui
{

struct menu_item
{
    int32_t id;

    std::string text;

    std::string hotkey;

    std::shared_ptr<image> image_;

    bool visible, disabled, has_child;

    std::function<void(int32_t)> click_callback;

    inline bool operator==(const int32_t id_)
    {
        return id == id_;
    }
};

class menu : public i_control, public std::enable_shared_from_this<menu>
{
public:
    menu(const std::string &text, std::shared_ptr<i_theme> theme_ = nullptr);
    ~menu();

    virtual void draw(graphic &gr);

    virtual void receive_event(const event &ev);

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

    void append_item(const menu_item &mi);
    void update_item(const menu_item &mi, int32_t id);
    void swap_items(int32_t first_item_id, int32_t second_item_id);
    void delete_item(int32_t id);

    void set_item_height(int32_t item_height);
    void set_max_width(int32_t width);

    void show_on_control(i_control &control, int32_t relative); /// If relative < 0 the menu showed on control

public:
    /// Control name in theme
    static constexpr const char *tc = "menu";

    /// Used theme values
    static constexpr const char *tv_background = "background";
    static constexpr const char *tv_border = "border";
    static constexpr const char *tv_text = "text";
    static constexpr const char *tv_selected = "selected";
    static constexpr const char *tv_select_active = "select_active";
    static constexpr const char *tv_round = "round";
    static constexpr const char *tv_font = "font";

private:
    std::shared_ptr<i_theme> theme_;

    rect position_;

    std::weak_ptr<window> parent;

    std::vector<menu_item> items;

    int32_t item_height;
    int32_t max_width;

    bool showed_;

    void update_size();

    void redraw();
};

}
