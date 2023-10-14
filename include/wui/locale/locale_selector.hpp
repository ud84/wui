//
// Copyright (c) 2023 Anton Golovkov (udattsk at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/ud84/wui
//

#pragma once

#include <wui/locale/locale_type.hpp>
#include <vector>

namespace wui
{

struct locale_params
{
    locale_type type;
    std::string name;
    std::string file_name;
    int32_t resource_id;

    inline bool operator==(const locale_params &lv)
    {
        return type == lv.type;
    }

    inline bool operator==(const locale_type &lt)
    {
        return type == lt;
    }

    std::string str()
    {
        return wui::str(type) + ", " + name + ", " + file_name + ", " + std::to_string(resource_id);
    }
};

using locales_t = std::vector<locale_params>;

void set_app_locales(const locales_t &);
void set_default_locale(locale_type);

locale_params get_app_locale(locale_type);

locale_type get_current_app_locale();
locale_type get_next_app_locale();

locale_type get_default_system_locale();

const locales_t &get_app_locales();

}
