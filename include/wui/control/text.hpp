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
#include <wui/common/rect.hpp>
#include <wui/common/color.hpp>

#include <string>
#include <functional>
#include <memory>
#include <vector>

namespace wui
{

enum class text_alignment
{
    left,
    center,
    right
};

class text : public i_control, public std::enable_shared_from_this<text>
{
public:
    text(std::string_view text = "", text_alignment alignment = text_alignment::left, std::string_view theme_control_name = tc, std::shared_ptr<i_theme> theme_ = nullptr);
    ~text();

    virtual void draw(graphic &gr, const rect &);

    virtual void set_position(const rect &position, bool redraw = true);
    virtual rect position() const;

    virtual void set_parent(std::shared_ptr<window> window_);
    virtual std::weak_ptr<window> parent() const;
    virtual void clear_parent();

    virtual void set_topmost(bool yes);
    virtual bool topmost() const;

    virtual void update_theme_control_name(std::string_view theme_control_name);
    virtual void update_theme(std::shared_ptr<i_theme> theme_ = nullptr);

    virtual void show();
    virtual void hide();
    virtual bool showed() const;

    virtual void enable();
    virtual void disable();
    virtual bool enabled() const;

    virtual bool focused() const;
    virtual bool focusing() const;

    virtual error get_error() const;

public:
    /// Text's interface
    void set_text(std::string_view text);
	std::string_view get_text() const;

    void set_alignment(text_alignment alignment);

public:
    /// Control name in theme
    static constexpr const char *tc = "text";

    /// Used theme values
    static constexpr const char *tv_color = "color";
    static constexpr const char *tv_font = "font";

private:
    std::string tcn; /// control name in theme
    std::shared_ptr<i_theme> theme_;

    rect position_;

    std::weak_ptr<window> parent_;

    bool showed_, topmost_;

    std::string text_;

    text_alignment alignment;
	
    void redraw();
};

}
