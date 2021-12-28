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

#ifdef _WIN32
#include <gdiplus.h>
#endif

namespace wui
{

class image : public i_control, public std::enable_shared_from_this<image>
{
public:
#ifdef _WIN32
    image(int32_t resource_index, std::shared_ptr<i_theme> theme_ = nullptr);
#endif
    image(const std::wstring &fileName, std::shared_ptr<i_theme> theme_ = nullptr);
    ~image();

    virtual void draw(graphic &gr);

    virtual void receive_event(const event &ev);

    virtual void set_position(const rect &position);
    virtual rect position() const;

    virtual void set_parent(std::shared_ptr<window> window_);
    virtual void clear_parent();

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

#ifdef _WIN32
    void change_image(int32_t resource_index);
#endif
    void change_image(const std::wstring &file_name);

    int32_t width() const;
    int32_t height() const;

private:
    std::shared_ptr<i_theme> theme_;

    rect position_;

    std::weak_ptr<window> parent;

    bool showed_;

    std::wstring file_name;
	
#ifdef _WIN32
    int32_t resource_index;
    Gdiplus::Image *img;
#endif

    void redraw();
};

}