//
// Copyright (c) 2021-2022 Anton Golovkov (udattsk at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/ud84/WUI
//

#include <wui/theme/theme_impl.hpp>

#include <nlohmann/json.hpp>
#include <boost/nowide/fstream.hpp>
#include <sstream>

namespace wui
{

theme_impl::theme_impl(const std::string &name_)
    : name(name_), ints(), strings(), fonts()
{
}

std::string theme_impl::get_name() const
{
    return name;
}

void theme_impl::set_color(const std::string &control, const std::string &value, color color_)
{
    ints[control + value] = color_;
}

color theme_impl::get_color(const std::string &control, const std::string &value) const
{
    auto it = ints.find(control + value);
    if (it != ints.end())
    {
        return static_cast<color>(it->second);
    }
    return 0;
}

void theme_impl::set_dimension(const std::string &control, const std::string &value, int32_t dimension)
{
    ints[control + value] = dimension;
}

int32_t theme_impl::get_dimension(const std::string &control, const std::string &value) const
{
    auto it = ints.find(control + value);
    if (it != ints.end())
    {
        return it->second;
    }
    return 0;
}

void theme_impl::set_string(const std::string &control, const std::string &value, const std::string &str)
{
    strings[control + value] = str;
}

std::string theme_impl::get_string(const std::string &control, const std::string &value) const
{
    auto it = strings.find(control + value);
    if (it != strings.end())
    {
        return it->second;
    }
    return "";
}

void theme_impl::set_font(const std::string &control, const std::string &value, const font &font_)
{
    fonts[control + value] = font_;
}

font theme_impl::get_font(const std::string &control, const std::string &value) const
{
    auto it = fonts.find(control + value);
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
    for (auto c = controls.begin(); c != controls.end(); ++c)
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
    boost::nowide::ifstream f(file_name);
    
    std::stringstream buffer;
    buffer << f.rdbuf();

    load_json(buffer.str());
}

void theme_impl::load_theme(const i_theme &theme_)
{
    ints = static_cast<const theme_impl*>(&theme_)->ints;
    strings = static_cast<const theme_impl*>(&theme_)->strings;
    fonts = static_cast<const theme_impl*>(&theme_)->fonts;
}

}
