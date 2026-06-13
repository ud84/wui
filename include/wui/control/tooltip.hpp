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
#include <wui/common/rect.hpp>
#include <wui/common/color.hpp>

#include <string>
#include <functional>
#include <memory>

namespace wui
{

class tooltip : public i_control, public std::enable_shared_from_this<tooltip>
{
public:
    tooltip(std::string_view text, std::string_view theme_control_name = tc, std::shared_ptr<i_theme> theme_ = nullptr);
    virtual ~tooltip();

    virtual void draw(graphic &gr, const rect& ) override;

    virtual void set_position(const rect& position) override;
    [[nodiscard]] virtual rect position() const override;

    virtual void set_parent(std::shared_ptr<window> window_) override;
    [[nodiscard]] virtual std::weak_ptr<window> parent() const override;
    virtual void clear_parent();

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
    /// Tooltip's interface
    void show_on_control(i_control &control, int32_t indent);

    void set_text(std::string_view text_);

    void redraw();

public:
    /// Control name in theme
    static constexpr const char *tc = "tooltip";

    /// Used theme values
    static constexpr const char *tv_background = "background";
    static constexpr const char *tv_border = "border";
    static constexpr const char *tv_border_width = "border_width";
    static constexpr const char *tv_text = "text";
    static constexpr const char *tv_text_indent = "text_indent";
    static constexpr const char *tv_round = "round";
    static constexpr const char *tv_font = "font";

private:
    std::string tcn; /// control name in theme
    std::shared_ptr<i_theme> theme_;

    std::string text;

    rect position_;

    std::weak_ptr<window> parent_;

    bool showed_;

    void update_size();

};

}
