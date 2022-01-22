//
// Copyright (c) 2021-2022 Anton Golovkov (udattsk at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/ud84/wui
//

#pragma once

#include <memory>

namespace wui
{

struct rect;
class graphic;
class window;
class i_theme;

class i_control
{
public:
    virtual void draw(graphic &gr) = 0;

    virtual void set_position(const rect &position) = 0;
    virtual rect position() const = 0;

    virtual void set_parent(std::shared_ptr<window> window_) = 0;
    virtual void clear_parent() = 0;

    virtual bool topmost() const = 0;

    virtual void set_focus() = 0;
    virtual bool remove_focus() = 0;   /// Returns false if the control changes focus within its internal controls
    virtual bool focused() const = 0;  /// Returns true if the control is focused
    virtual bool focusing() const = 0; /// Returns true if the control receives focus

    virtual void update_theme(std::shared_ptr<i_theme> theme_ = nullptr) = 0;

    virtual void show() = 0;
    virtual void hide() = 0;
    virtual bool showed() const = 0;

    virtual void enable() = 0;
    virtual void disable() = 0;
    virtual bool enabled() const = 0;

protected:
    ~i_control() {}

};

}
