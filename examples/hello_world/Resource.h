//
// Copyright (c) 2021-2022 Anton Golovkov (udattsk at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/ud84/wui
//

static constexpr const int dark_theme_index = 0;
static constexpr const int light_theme_index = 1;

#ifdef _WIN32

#define IDI_MAIN_ICON           107
#define IMG_LOGO				109

#define IMG_ACCOUNT     		110
#define IMG_MENU    		    111

#define TXT_DARK_THEME          200
#define TXT_LIGHT_THEME         201

#define TXT_LOCALE_EN           210
#define TXT_LOCALE_RU           211

#else // _WIN32

static constexpr const char * config_ini_file       = "hello_world.ini";

static constexpr const char * light_theme_json_file = "res/light.json";
static constexpr const char * dark_theme_json_file  = "res/dark.json";

static constexpr const char* IMG_LOGO               = "logo.png";
static constexpr const char* IMG_ACCOUNT            = "account.png";
static constexpr const char* IMG_MENU               = "menu.png";

#endif
