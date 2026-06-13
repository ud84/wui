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
#include <vector>

#ifdef _WIN32
#include <gdiplus.h>
#elif __linux__
#include <cairo/cairo.h>
#endif

namespace wui
{

class image : public i_control, public std::enable_shared_from_this<image>
{
public:
#ifdef _WIN32
    image(int32_t resource_index, std::shared_ptr<i_theme> theme_ = nullptr);
#endif
    image(std::string_view file_name, std::shared_ptr<i_theme> theme_ = nullptr);
    image(const std::vector<uint8_t> &data);

    virtual ~image();

    virtual void draw(graphic &gr, const rect& ) override;

    virtual void set_position(const rect& position) override;
    [[nodiscard]] virtual std::weak_ptr<window> parent() const override;
    [[nodiscard]] virtual rect position() const override;

    virtual void set_parent(std::shared_ptr<window> window_) override;
    virtual void clear_parent() override;

    virtual void set_topmost(bool yes) noexcept override;
    [[nodiscard]] virtual bool topmost() const noexcept override;

    virtual void update_theme_control_name(std::string_view theme_control_name) override;
    virtual void update_theme(std::shared_ptr<i_theme> theme_ = nullptr) override;

    virtual void show() override;
    virtual void hide() override;
    [[nodiscard]] virtual bool showed() const override;

    virtual void enable() override;
    virtual void disable();
    [[nodiscard]] virtual bool enabled() const override;

    [[nodiscard]] virtual bool focused() const noexcept override;
    [[nodiscard]] virtual bool focusing() const noexcept override;

    [[nodiscard]] virtual error get_error() const override;

public:
    /// Image's interface
#ifdef _WIN32
    void change_image(const int32_t resource_index);
#endif

    void change_image(std::string_view file_name);
    void change_image_raw(std::string_view data_name_, std::shared_ptr<i_theme> theme__ = nullptr);
    void change_image(const std::vector<uint8_t> &data);

    [[nodiscard]] int32_t width() const;
    [[nodiscard]] int32_t height() const;

    void redraw();

public:
    /// Control name in theme
    static constexpr const char *tc = "image";

    /// Used theme values
    static constexpr const char *tv_resource = "resource";
    static constexpr const char *tv_path = "path";

private:
    std::shared_ptr<i_theme> theme_;

    rect position_;

    std::weak_ptr<window> parent_;

    bool showed_, topmost_;

    std::string file_name;
    std::string path_;
    std::string data_name;
    std::string theme_name;

#ifdef _WIN32
    int32_t resource_index;
    Gdiplus::Image *img;
#elif __linux__
    cairo_surface_t *img;
#endif

    error err;
};

}
