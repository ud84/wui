//
// Copyright (c) 2021-2022 Anton Golovkov (udattsk at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/ud84/wui
//

#pragma once

#include <wui/theme/i_theme.hpp>

#include <memory>

namespace wui
{

/// Set and get the current theme
#ifdef _WIN32
bool set_default_theme_from_resource(const std::string &name, int32_t resource_index, const std::string &resource_section);
#endif
bool set_default_theme_from_json(const std::string &name, const std::string &json);
bool set_default_theme_from_file(const std::string &name, const std::string &file_name);
void set_default_theme_empty(const std::string &name);

/// Load theme from regsitry on Windows or from file on other systems
/// Parameters are setted by set_app_themes() in theme_selector.hpp
bool set_default_theme_from_name(const std::string &name);

/// Return the pointer to current default theme instance
std::shared_ptr<i_theme> get_default_theme();

/// Make the custom theme for the some control
std::shared_ptr<i_theme> make_custom_theme(const std::string &name = "");
std::shared_ptr<i_theme> make_custom_theme(const std::string &name, const std::string &json);

/// Return the item's color by current theme
color theme_color(const std::string &control, const std::string &value, std::shared_ptr<i_theme> theme_ = nullptr);

/// Return the item's dimension by current theme
int32_t theme_dimension(const std::string &control, const std::string &value, std::shared_ptr<i_theme> theme_ = nullptr);

/// Return the item's string value by current theme
const std::string &theme_string(const std::string &control, const std::string &value, std::shared_ptr<i_theme> theme_ = nullptr);

/// Return the item's font value by current theme
font theme_font(const std::string &control, const std::string &value, std::shared_ptr<i_theme> theme_ = nullptr);

const std::vector<uint8_t> &theme_image(const std::string &name, std::shared_ptr<i_theme> theme_ = nullptr);

}
