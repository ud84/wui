
#pragma once

#ifdef _WIN32

#define IDI_SIMPLE			    107
#define IMG_ACCOUNT				109
#define IMG_SETTINGS    		110

#define TXT_DARK_THEME          200
#define TXT_LIGHT_THEME         201

#define TXT_LOCALE_EN           210
#define TXT_LOCALE_RU           211

#else

static int TXT_LOCALE_EN = 0, TXT_LOCALE_RU = 0, TXT_DARK_THEME = 0, TXT_LIGHT_THEME = 0;

const constexpr char *IMG_ACCOUNT = "res/images/dark/account.png";
const constexpr char *IMG_SETTINGS = "res/images/dark/settings.png";

#endif
