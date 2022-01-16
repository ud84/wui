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

#include <string>
#include <functional>
#include <memory>

#ifdef __linux__
#endif

namespace wui
{

class image;
class tooltip;

enum class button_view
{
    only_text,
    only_image,
    image_right_text,
    image_bottom_text,
    image_right_text_no_frame
};

class button : public i_control, public std::enable_shared_from_this<button>
{
public:
    button(const std::string &caption, std::function<void(void)> click_callback, std::shared_ptr<i_theme> theme_ = nullptr);

#ifdef _WIN32
    button(const std::string &caption, std::function<void(void)> click_callback, button_view button_view_, int32_t image_resource_index, int32_t image_size, std::shared_ptr<i_theme> theme_ = nullptr);
#endif
    button(const std::string &caption, std::function<void(void)> click_callback, button_view button_view_, const std::string &image_file_name, int32_t image_size, std::shared_ptr<i_theme> theme_ = nullptr);
    ~button();

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

    void set_caption(const std::string &caption);

    void set_button_view(button_view button_view_);
#ifdef _WIN32
    void set_image(int32_t resourceIndex);
#endif
    void set_image(const std::string &fileName);

    void enable_focusing();
    void disable_focusing();

    void set_callback(std::function<void(void)> click_callback);

public:
    /// Control name in theme
    static constexpr const char *tc = "button";

    /// Used theme values
    static constexpr const char *tv_calm = "calm";
    static constexpr const char *tv_active = "active";
    static constexpr const char *tv_border = "border";
    static constexpr const char *tv_focused_border = "focused_border";
    static constexpr const char *tv_text = "text";
    static constexpr const char *tv_disabled = "disabled";
    static constexpr const char *tv_round = "round";
    static constexpr const char *tv_font = "font";

private:
    button_view button_view_;
    std::string caption;
    std::shared_ptr<image> image_;
    int32_t image_size;
    std::shared_ptr<tooltip> tooltip_;
    std::function<void(void)> click_callback;
    std::shared_ptr<i_theme> theme_;

    rect position_;

    std::weak_ptr<window> parent;

    bool showed_, enabled_;
    bool active, focused_;
    bool focusing_;

    void redraw();
};

}
