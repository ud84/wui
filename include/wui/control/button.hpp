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

#include <string>
#include <functional>
#include <memory>
#include <vector>

namespace wui
{

class image;
class tooltip;

enum class button_view
{
    text,
    image,
    image_right_text,
    image_bottom_text,
    switcher,
    radio,
    anchor,
    sheet
};

class button : public i_control, public std::enable_shared_from_this<button>
{
public:
    button(std::string_view caption, std::function<void(void)> click_callback, std::string_view theme_control_name = tc, std::shared_ptr<i_theme> theme_ = nullptr);
    button(std::string_view caption, std::function<void(void)> click_callback, button_view button_view_, std::string_view theme_control_name = tc, std::shared_ptr<i_theme> theme_ = nullptr);

#ifdef _WIN32
    button(std::string_view caption, std::function<void(void)> click_callback, button_view button_view_, int32_t image_resource_index, int32_t image_size, std::string_view theme_control_name = tc, std::shared_ptr<i_theme> theme_ = nullptr);
#endif
    button(std::string_view caption, std::function<void(void)> click_callback, button_view button_view_, std::string_view image_file_name, int32_t image_size, std::string_view theme_control_name = tc, std::shared_ptr<i_theme> theme_ = nullptr);
    button(std::string_view caption, std::function<void(void)> click_callback, button_view button_view_, const std::vector<uint8_t> &image_data, int32_t image_size, std::string_view theme_control_name = tc, std::shared_ptr<i_theme> theme_ = nullptr);
    virtual ~button();

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
    /// Button's interface
    void set_caption(std::string_view caption);

    void set_button_view(button_view button_view_);
#ifdef _WIN32
    void set_image(int32_t resource_index);
#endif
    void set_image(std::string_view file_name);
    void set_image(const std::vector<uint8_t> &image_data);

    void enable_focusing();
    void disable_focusing();

    void turn(bool on);
    [[nodiscard]] bool turned() const;
    [[nodiscard]] rect get_preferred_size();

    void set_callback(std::function<void(void)> click_callback);
    void set_callback_down(std::function<void(void)> click_callback);

    void redraw();

public:
    /// Possible control names in theme
    static constexpr const char *tc = "button";
    static constexpr const char *tc_tool = "tool_button";
    static constexpr const char *tc_tool_red = "red_tool_button";

    /// Used theme values
    static constexpr const char *tv_calm = "calm";
    static constexpr const char *tv_active = "active";
    static constexpr const char *tv_border = "border";
    static constexpr const char *tv_border_width = "border_width";
    static constexpr const char *tv_hover_border = "hover_border";
    static constexpr const char *tv_focused_border = "focused_border";
    static constexpr const char *tv_text = "text";
    static constexpr const char *tv_disabled = "disabled";
    static constexpr const char *tv_anchor = "anchor";
    static constexpr const char *tv_round = "round";
    static constexpr const char *tv_focusing = "focusing";
    static constexpr const char *tv_font = "font";

    ///Used theme images
    static constexpr const char *ti_switcher_off = "button_switcher_off";
    static constexpr const char *ti_switcher_on = "button_switcher_on";
    static constexpr const char *ti_radio_off = "button_radio_off";
    static constexpr const char *ti_radio_on = "button_radio_on";

private:
    button_view button_view_;
    std::string caption;
    std::string caption_org;

    std::shared_ptr<image> image_;
    int32_t image_size;

    std::shared_ptr<tooltip> tooltip_;

    std::function<void(void)> click_callback;
    std::function<void(void)> click_callback_down{};

    std::string tcn; /// control name in theme
    std::shared_ptr<i_theme> theme_;

    rect position_;

    std::weak_ptr<window> parent_;
    std::string my_subscriber_id;

    bool showed_, enabled_, topmost_;
    bool active, focused_;
    bool focusing_;

    bool pushed;

    bool turned_;

    rect text_rect_;

    error err;

    void receive_event(const event &ev);

    void update_err(std::string_view place, const error &input_err);
};

}
