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

#include <string>
#include <functional>
#include <memory>

namespace wui
{

class tooltip : public i_control, public std::enable_shared_from_this<tooltip>
{
public:
    tooltip(const std::wstring &text, std::shared_ptr<i_theme> theme_ = nullptr);
    ~tooltip();

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

    void set_text(const std::wstring &text_);

private:
    std::shared_ptr<i_theme> theme_;

    std::wstring text;

    rect position_;

    std::weak_ptr<window> parent;

    bool showed_;

    void update_size();

    void redraw();
};

}
