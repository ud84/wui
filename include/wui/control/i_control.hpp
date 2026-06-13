//
// Copyright (c) 2021-2026 Intent Garden Org
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
//

#pragma once

#include <wui/common/error.hpp>

#include <memory>
#include <string>

namespace wui
{

struct rect;
class graphic;
class window;
class i_theme;

class i_control
{
public:
    virtual void draw(graphic &gr, const rect& paint_rect) = 0;

    virtual void set_position(const rect& position) = 0;
    [[nodiscard]] virtual rect position() const = 0;

    virtual void set_parent(std::shared_ptr<window> window_) = 0;
    [[nodiscard]] virtual std::weak_ptr<window> parent() const = 0;
    virtual void clear_parent() = 0;

    virtual void set_topmost(bool yes) = 0;
    [[nodiscard]] virtual bool topmost() const = 0;

    virtual void update_theme_control_name(std::string_view theme_control_name) = 0;
    virtual void update_theme(std::shared_ptr<i_theme> theme_ = nullptr) = 0;

    virtual void show() = 0;
    virtual void hide() = 0;
    [[nodiscard]] virtual bool showed() const = 0;

    virtual void enable() = 0;
    virtual void disable() = 0;
    [[nodiscard]] virtual bool enabled() const = 0;

    [[nodiscard]] virtual bool focused() const = 0;  /// Returns true if the control is focused
    [[nodiscard]] virtual bool focusing() const = 0; /// Returns true if the control receives focus

    [[nodiscard]] virtual error get_error() const = 0;

    friend class window;

protected:
    virtual ~i_control() {}

};

}
