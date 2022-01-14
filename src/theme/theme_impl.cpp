//
// Copyright (c) 2021-2022 Anton Golovkov (udattsk at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/ud84/WUI
//

#include <wui/theme/theme_impl.hpp>
#include <wui/theme/theme_str.hpp>

#include <nlohmann/json.hpp>

namespace wui
{

theme_impl::theme_impl(theme theme__)
    : theme_(theme__), ints(), strings(), fonts()
{
}

theme theme_impl::get_theme() const
{
    return theme_;
}

void theme_impl::set_color(theme_control control, theme_value value, color color_)
{
    ints[theme_control_to_str(control) + theme_value_to_str(value)] = color_;
}

color theme_impl::get_color(theme_control control, theme_value value) const
{
    auto it = ints.find(theme_control_to_str(control) + theme_value_to_str(value));
    if (it != ints.end())
    {
        return static_cast<color>(it->second);
    }
    return 0;
}

void theme_impl::set_dimension(theme_control control, theme_value value, int32_t dimension)
{
    ints[theme_control_to_str(control) + theme_value_to_str(value)] = dimension;
}

int32_t theme_impl::get_dimension(theme_control control, theme_value value) const
{
    auto it = ints.find(theme_control_to_str(control) + theme_value_to_str(value));
    if (it != ints.end())
    {
        return it->second;
    }
    return 0;
}

void theme_impl::set_string(theme_control control, theme_value value, const std::string &str)
{
    strings[theme_control_to_str(control) + theme_value_to_str(value)] = str;
}

std::string theme_impl::get_string(theme_control control, theme_value value) const
{
    auto it = strings.find(theme_control_to_str(control) + theme_value_to_str(value));
    if (it != strings.end())
    {
        return it->second;
    }
    return "";
}

void theme_impl::set_font(theme_control control, theme_value value, const font &font_)
{
    fonts[theme_control_to_str(control) + theme_value_to_str(value)] = font_;
}

font theme_impl::get_font(theme_control control, theme_value value) const
{
    auto it = fonts.find(theme_control_to_str(control) + theme_value_to_str(value));
    if (it != fonts.end())
    {
        return it->second;
    }
    return font();
}

void theme_impl::load_json(const std::string &json_)
{
    auto j = nlohmann::json::parse(json_);

    auto controls = j.at("controls");
    for (auto &c = controls.begin(); c != controls.end(); ++c)
    {
        std::string control = c->at("type").get<std::string>();

        auto obj = c->get<nlohmann::json::object_t>();
        for (auto& kvp : obj)
        {
            if (kvp.first == "type")
            {
                continue;
            }

            if (kvp.second.is_string())
            {
                auto str = kvp.second.get<std::string>();
                if (str[0] == '#')
                {
                    try
                    {
                        str.erase(0, 1);
                        str.insert(0, "0x");
                        int32_t color = std::stol(str, nullptr, 16);

                        ints[control + kvp.first] = make_color(get_red(color), get_green(color), get_blue(color));
                    }
                    catch (...)
                    {
                        fprintf(stderr, "Error reading color in control: %s, key: %s, value: %s \n", control.c_str(), kvp.first.c_str(), str.c_str());
                    }
                }
                else
                {
                    strings[control + kvp.first] = str;
                }
            }
            else if (kvp.second.is_number_integer())
            {
                ints[control + kvp.first] = kvp.second.get<int32_t>();
            }
            else if (kvp.second.is_object())
            {
                auto fnt = kvp.second.get<nlohmann::json::object_t>();
                fonts[control + kvp.first] = font{ fnt.at("name").get<std::string>(), fnt.at("size").get<int32_t>(), decorations::normal };
            }
        }
    }
}

void theme_impl::load_file(const std::string &file_name)
{

}

}
