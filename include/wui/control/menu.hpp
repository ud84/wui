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

#include <wui/control/list.hpp>

#include <string>
#include <vector>
#include <functional>
#include <memory>

namespace wui
{

class image;
//class menu;

enum class menu_item_state
{
    normal,
    separator,
    expanded,
    disabled
};

template < typename T >
struct menu_item_data_
{
    int32_t id{ };

    menu_item_state state{ menu_item_state::normal };

    std::string text;

    std::string hotkey;

    std::shared_ptr<image> image_{ };

    std::vector<T> children;

    std::function<void(int32_t)> click_callback{ };

    inline bool operator==(int32_t id_)
    {
        return id == id_;
    }

    menu_item_data_& operator=(const menu_item_data_& item)
    {
        id = item.id;
        state = item.state;
        text = item.text;
        hotkey = item.hotkey;
        image_ = item.image_;
        click_callback = item.click_callback;
        return *this;
    }
};

struct menu_item;

struct menu_item_data : public menu_item_data_ < menu_item_data >
{
    menu_item_data& operator=(const menu_item_data& item)
    {
        menu_item_data_ < menu_item_data >::operator =(item);
        return *this;
    }
};

struct menu_item : public menu_item_data_ < menu_item >
{
    int32_t level;
    menu_item_state prev_state;

    inline menu_item& operator=(const menu_item& item)
    {
        menu_item_data_ < menu_item >::operator =(item);
        return *this;
    }

    inline menu_item& operator=(const menu_item_data& item)
    {
        id = item.id;
        state = item.state;
        text = item.text;
        hotkey = item.hotkey;
        image_ = item.image_;
        click_callback = item.click_callback;
        return *this;
    }
};

typedef std::vector<menu_item_data> menu_items_t_;
typedef std::vector<menu_item> menu_items_t;

class menu : public i_control, public std::enable_shared_from_this<menu>
{
public:
    menu(std::string_view theme_control_name = tc, std::shared_ptr<i_theme> theme_ = nullptr);
    virtual ~menu();

    virtual void draw(graphic &gr, const rect&) override;

    virtual void set_position(const rect& position) override;
    [[nodiscard]] virtual rect position() const override;

    virtual void set_parent(std::shared_ptr<window> window_) override;
    [[nodiscard]] virtual std::weak_ptr<window> parent() const override;
    virtual void clear_parent() override;

    virtual void set_topmost(bool) override;
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
    /// Menu's interface
    //void set_items(const menu_items_t &mi);
    void set_items(const menu_items_t_&mi);
    void update_item(const menu_item &mi);
    void swap_items(int32_t first_item_id, int32_t second_item_id);
    void delete_item(int32_t id);

    void set_item_height(int32_t item_height) noexcept;

    void show_on_control(std::shared_ptr<i_control> control, int32_t indent, int32_t x = -1, int32_t y = -1);

    void show_on_point(int32_t x, int32_t y);

    [[nodiscard]] bool is_showed() const
    {
        return showed_ || list_->showed();
    }

public:
    /// Control name in theme
    static constexpr const char *tc = "menu";

    /// Used theme values
    static constexpr const char *tv_background = "background";
    static constexpr const char *tv_border = "border";
    static constexpr const char *tv_border_width = "border_width";
    static constexpr const char *tv_text = "text";
    static constexpr const char *tv_disabled_text = "disabled_text";
    static constexpr const char *tv_selected_item = "selected_item";
    static constexpr const char *tv_scrollbar = "scrollbar";
    static constexpr const char *tv_scrollbar_slider = "scrollbar_slider";
    static constexpr const char *tv_scrollbar_slider_acive = "scrollbar_slider_active";
    static constexpr const char *tv_round = "round";
    static constexpr const char *tv_font = "font";

private:
    std::shared_ptr<i_theme> list_theme;
    std::shared_ptr<list> list_;

    std::string tcn; /// control name in theme
    std::shared_ptr<i_theme> theme_;

    rect position_;

    std::weak_ptr<window> parent_;
    std::string my_subscriber_id;

    std::shared_ptr<i_control> activation_control;
    int32_t indent, x, y;

    std::vector<menu_item> items;

    int32_t max_text_width, max_hotkey_width;

    int32_t item_height_;

    bool showed_;
    bool size_updated;

    void update_list_theme();

    void receive_event(const event &ev);

    void update_size();

    void draw_arrow_down(graphic &gr, const rect& pos, const bool expanded);
    void draw_list_item(wui::graphic &gr, const int32_t n_item, const rect& item_rect_,
        const list::item_state state);
    void activate_list_item(const int32_t n_item);
};

}
