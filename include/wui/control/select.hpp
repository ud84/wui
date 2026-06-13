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
#include <functional>
#include <memory>
#include <vector>

namespace wui
{

struct select_item;

typedef std::vector<select_item> select_items_t;

struct select_item
{
    int64_t id;

    std::string text;

    inline bool operator==(int64_t id_)
    {
        return id == id_;
    }
};

class select : public i_control, public std::enable_shared_from_this<select>
{
public:
    select(std::string_view theme_control_name = tc, std::shared_ptr<i_theme> theme_ = nullptr);
    virtual ~select();

    virtual void draw(graphic &gr, const rect&) override;

    virtual void set_position(const rect& position) override;
    [[nodiscard]] virtual rect position() const override;

    virtual void set_parent(std::shared_ptr<window> window_) override;
    [[nodiscard]] virtual std::weak_ptr<window> parent() const override;
    virtual void clear_parent();

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

    /// Select's interface
    void set_items(const select_items_t &items);
    void update_item(const select_item &mi);
    void swap_items(const int64_t first_item_id, const int64_t second_item_id);
    void delete_item(const int64_t id);

    void set_item_height(const int32_t item_height) noexcept;

    void select_item_number(const int32_t index);
    void select_item_id(const int64_t id);
    [[nodiscard]] select_item selected_item() const;
    [[nodiscard]] const select_items_t &items() const;

    void set_change_callback(std::function<void(int32_t /* number */, int64_t /* id */)> change_callback) noexcept;
    [[nodiscard]] int32_t get_font_size() const;

    void redraw();

public:
    /// Control name in theme
    static constexpr const char *tc = "select";

    /// Used theme values
    static constexpr const char *tv_background = "background";
    static constexpr const char *tv_border = "border";
    static constexpr const char *tv_border_width = "border_width";
    static constexpr const char *tv_hover_border = "hover_border";
    static constexpr const char *tv_focused_border = "focused_border";
    static constexpr const char *tv_button_calm = "button_calm";
    static constexpr const char *tv_button_active = "button_active";
    static constexpr const char *tv_text = "text";
    static constexpr const char *tv_scrollbar = "scrollbar";
    static constexpr const char *tv_scrollbar_slider = "scrollbar_slider";
    static constexpr const char *tv_scrollbar_slider_acive = "scrollbar_slider_active";
    static constexpr const char *tv_selected_item = "selected_item";
    static constexpr const char *tv_active_item = "active_item";
    static constexpr const char *tv_round = "round";
    static constexpr const char *tv_font = "font";

private:
    std::vector<select_item> items_;

    std::function<void(int32_t, int64_t)> change_callback;

    std::string tcn; /// control name in theme
    std::shared_ptr<i_theme> theme_;

    rect position_;

    std::weak_ptr<window> parent_;
    std::string my_control_sid, my_plain_sid;

    std::shared_ptr<i_theme> list_theme;
    std::shared_ptr<list> list_;

    bool showed_, enabled_, active, topmost_;
    bool focused_;
    bool focusing_;

    int32_t left_shift;

    int32_t item_height_;

    void receive_control_events(const event &ev);
    void receive_plain_events(const event &ev);

    void update_list_theme();

    void draw_arrow_down(graphic &gr, const rect &pos);

    void select_up();
    void select_down();

    void show_list();

    void draw_list_item(graphic &gr, const int32_t n_item, const rect &item_rect_, const list::item_state state);
    void activate_list_item(const int32_t n_item);
    void change_list_item(const int32_t n_item);
};

}
