//
// Copyright (c) 2021-2022 Anton Golovkov (udattsk at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/ud84/wui
//

#include <wui/locale/locale_impl.hpp>
#include <wui/system/tools.hpp>
#include <wui/system/path_tools.hpp>

#include <nlohmann/json.hpp>
#include <boost/nowide/convert.hpp>

#include <sstream>
#include <fstream>
#include <iostream>

#ifdef _WIN32
#include <windows.h>
#endif

namespace wui
{

locale_impl::locale_impl(locale_type type_, const std::string &name_)
    : type(type_), name(name_), strings(), dummy_string(), ok(false)
{
}

locale_type locale_impl::get_type() const
{
    return type;
}

std::string locale_impl::get_name() const
{
    return name;
}

void locale_impl::set(const std::string &control, const std::string &value, const std::string &str)
{
    strings[control + value] = str;
}

const std::string &locale_impl::get(const std::string &control, const std::string &value) const
{
    auto it = strings.find(control + value);
    if (it != strings.end())
    {
        return it->second;
    }
    return dummy_string;
}

#ifdef _WIN32
void locale_impl::load_resource(int32_t resource_index, const std::string &resource_section)
{
    auto h_inst = GetModuleHandle(NULL);
    auto h_resource = FindResource(h_inst, MAKEINTRESOURCE(resource_index), boost::nowide::widen(resource_section).c_str());
    if (!h_resource)
    {
        return;
    }

    auto resource_size = ::SizeofResource(h_inst, h_resource);
    if (!resource_size)
    {
        return;
    }

    const void* resource_data = ::LockResource(::LoadResource(h_inst, h_resource));
    if (!resource_data)
    {
        return;
    }

    load_json(std::string(static_cast<const char*>(resource_data), resource_size));
}
#endif

void locale_impl::load_json(const std::string &json_)
{
    try
    {
        auto j = nlohmann::json::parse(json_);

        if (j.is_discarded())
        {
            ok = false;
            return;
        }

        ok = true;

        auto sections = j.at("sections");
        for (auto &s : sections)
        {
            std::string section = s.at("type").get<std::string>();

            auto obj = s.get<nlohmann::json::object_t>();
            for (auto& kvp : obj)
            {
                if (kvp.first == "type")
                {
                    continue;
                }

                if (kvp.second.is_string())
                {
                    auto str = kvp.second.get<std::string>();
                    strings[section + kvp.first] = str;
                }
            }
        }
    }
    catch (nlohmann::detail::exception &e)
    {
        std::cerr << "WUI error :: Error reading locale json: " << e.what() << std::endl;
        ok = false;
    }
}

void locale_impl::load_file(const std::string &file_name)
{
    std::ifstream f(wui::real_path(file_name));
    if (!f)
    {
        std::cerr << "WUI error :: Unable to open locale file: " << wui::real_path(file_name) << " errno: " << errno << std::endl;
        return;
    }
    
    std::stringstream buffer;
    buffer << f.rdbuf();

    load_json(buffer.str());
}

void locale_impl::load_locale(const i_locale &locale_)
{
    strings = static_cast<const locale_impl*>(&locale_)->strings;
}

bool locale_impl::is_ok() const
{
    return ok;
}

}
