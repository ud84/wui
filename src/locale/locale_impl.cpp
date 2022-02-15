//
// Copyright (c) 2021-2022 Anton Golovkov (udattsk at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/ud84/wui
//

#include <wui/locale/locale_impl.hpp>

#include <nlohmann/json.hpp>
#include <boost/nowide/fstream.hpp>
#include <sstream>

#ifdef _WIN32
#include <windows.h>
#endif

namespace wui
{

locale_impl::locale_impl(const std::string &name_)
    : name(name_), strings(), dummy_string()
{
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
    HINSTANCE h_inst = GetModuleHandle(NULL);
    HRSRC h_resource = FindResource(h_inst, MAKEINTRESOURCE(resource_index), boost::nowide::widen(resource_section).c_str());
    if (!h_resource)
    {
        return;
    }

    DWORD image_size = ::SizeofResource(h_inst, h_resource);
    if (!image_size)
    {
        return;
    }

    const void* resource_data = ::LockResource(::LoadResource(h_inst, h_resource));
    if (!resource_data)
    {
        return;
    }

    load_json(static_cast<const char*>(resource_data));
}
#endif

void locale_impl::load_json(const std::string &json_)
{
    auto j = nlohmann::json::parse(json_);

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

void locale_impl::load_file(const std::string &file_name)
{
    boost::nowide::ifstream f(file_name);
    
    std::stringstream buffer;
    buffer << f.rdbuf();

    load_json(buffer.str());
}

void locale_impl::load_locale(const i_locale &locale_)
{
    strings = static_cast<const locale_impl*>(&locale_)->strings;
}

}
