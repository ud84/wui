//
// config.cpp - Contains the configuration wrappers impl
//
// Copyright (c) 2014-2023 Anton Golovkov (udattsk at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/ud84/wui
//

#include <wui/config/config.h>

#ifndef _WIN32
#include <inipp/inipp.h>

#include <fstream>
#include <sstream>

#else

#include <boost/nowide/convert.hpp>

#include <atlbase.h>
#include <atltrace.h>
#endif

#include <string>
#include <iostream>

#define PRINT_OPEN_ERROR std::cerr << "WUI Error :: Error opening config file: " << configFile << std::endl;

namespace wui
{

namespace config
{

#ifndef _WIN32

int32_t get_int(const std::string &section, const std::string &entry, int32_t default_, const std::string &configFile, int)
{
	inipp::Ini<char> ini;
	std::ifstream f(configFile);
	if (!f)
	{
		PRINT_OPEN_ERROR
		return default_;
	}
	ini.parse(f);

	int32_t value = default_;
	inipp::get_value(ini.sections[section], entry, value);

	return value;
}

void set_int(const std::string &section, const std::string &entry, int32_t value, const std::string &configFile, int)
{
	set_string(section, entry, std::to_string(value), configFile);
}

int64_t get_int64(const std::string &section, const std::string &entry, int64_t default_, const std::string &configFile, int)
{
	inipp::Ini<char> ini;
	std::ifstream f(configFile);
	if (!f)
	{
		PRINT_OPEN_ERROR
		return default_;
	}
	ini.parse(f);

	int64_t value = default_;
	inipp::get_value(ini.sections[section], entry, value);

	return value;
}

void set_int64(const std::string &section, const std::string &entry, int64_t value, const std::string &configFile, int)
{
	set_string(section, entry, std::to_string(value), configFile);
}

std::string get_string(const std::string &section, const std::string &entry, const std::string &default_, const std::string &configFile, int)
{
	inipp::Ini<char> ini;
	std::ifstream f(configFile);
	if (!f)
	{
		PRINT_OPEN_ERROR
		return default_;
	}
	ini.parse(f);

	std::string value = default_;
	inipp::get_value(ini.sections[section], entry, value);

	return value;
}

void set_string(const std::string &section_, const std::string &entry_, const std::string value_, const std::string &configFile, int)
{
	/*std::string name = section_ + "." + entry_;

	try
	{
		boost::property_tree::ptree pt;
		boost::property_tree::ini_parser::read_ini(configFile, pt);
		pt.put<std::string>(name, value_);
		boost::property_tree::write_ini(configFile, pt);
	}
	catch (const std::exception& e)
	{
		DBGTRACE("Error writing config: %s", e.what());
	}*/
}

void delete_value(const std::string &section_, const std::string &entry_, const std::string &configFile, int)
{
	set_string(section_, entry_, "", configFile);
}

void delete_key(const std::string &, const std::string &, int)
{
	// not impl
}

#else

int32_t get_int(const std::string &section, const std::string &entry, int32_t default_, const std::string &regKey, HKEY root)
{
	std::string strKey = regKey + "\\" + section;

	ATL::CRegKey key;

	if (key.Create(root, boost::nowide::widen(strKey).c_str(), REG_NONE, REG_OPTION_NON_VOLATILE, KEY_READ) == ERROR_SUCCESS)
	{
		DWORD dwVal;
		if (key.QueryDWORDValue(boost::nowide::widen(entry).c_str(), dwVal) == ERROR_SUCCESS)
		{
			key.Close();
			return dwVal;
		}
	}

	key.Close();
	return default_;
}

void set_int(const std::string &section, const std::string &entry, int32_t value, const std::string &regKey, HKEY root)
{
	std::string strKey = regKey + "\\" + section;

	ATL::CRegKey key;

	if (key.Create(root, boost::nowide::widen(strKey).c_str(), REG_NONE, REG_OPTION_NON_VOLATILE, KEY_WRITE) == ERROR_SUCCESS)
	{
		key.SetDWORDValue(boost::nowide::widen(entry).c_str(), value);
	}

	key.Close();
}

int64_t get_int64(const std::string &section, const std::string &entry, int64_t default_, const std::string &regKey, HKEY root)
{
	std::string strKey = regKey + "\\" + section;

	ATL::CRegKey key;

	if (key.Create(root, boost::nowide::widen(strKey).c_str(), REG_NONE, REG_OPTION_NON_VOLATILE, KEY_READ) == ERROR_SUCCESS)
	{
		ULONGLONG dwVal;
		if (key.QueryQWORDValue(boost::nowide::widen(entry).c_str(), dwVal) == ERROR_SUCCESS)
		{
			key.Close();
			return dwVal;
		}
	}

	key.Close();
	return default_;
}

void set_int64(const std::string &section, const std::string &entry, int64_t value, const std::string &regKey, HKEY root)
{
	std::string strKey = regKey + "\\" + section;

	ATL::CRegKey key;

	if (key.Create(root, boost::nowide::widen(strKey).c_str(), REG_NONE, REG_OPTION_NON_VOLATILE, KEY_WRITE) == ERROR_SUCCESS)
	{
		key.SetQWORDValue(boost::nowide::widen(entry).c_str(), value);
	}

	key.Close();
}

std::string get_string(const std::string &section, const std::string &entry, const std::string &default_, const std::string &regKey, HKEY root)
{
	std::string strKey = regKey + "\\" + section;

	std::string out = default_;

	ATL::CRegKey key;

	if (key.Create(root, boost::nowide::widen(strKey).c_str(), REG_NONE, REG_OPTION_NON_VOLATILE, KEY_READ) == ERROR_SUCCESS)
	{
		DWORD dwcbNeeded = 0;
		if (key.QueryStringValue(boost::nowide::widen(entry).c_str(), (LPTSTR)NULL, &dwcbNeeded) == ERROR_SUCCESS)
		{
			if (dwcbNeeded > 2048)
			{
				ATLTRACE(L"Config [E] too big string in: %s, %s, %s\n", regKey.c_str(), section.c_str(), entry.c_str());
				return default_;
			}

			wchar_t *val = (wchar_t*)calloc(dwcbNeeded, sizeof(wchar_t));

			if (key.QueryStringValue(boost::nowide::widen(entry).c_str(), val, &dwcbNeeded) == ERROR_SUCCESS)
			{
				out.clear();
				out.append(boost::nowide::narrow(val, dwcbNeeded - 1));
			}

			free(val);
		}
	}

	key.Close();

	return out;
}

void set_string(const std::string &section, const std::string &entry, const std::string value, const std::string &regKey, HKEY root)
{
	std::string strKey = regKey + "\\" + section;

	ATL::CRegKey key;

	if (key.Create(root, boost::nowide::widen(strKey).c_str(), REG_NONE, REG_OPTION_NON_VOLATILE, KEY_WRITE) == ERROR_SUCCESS)
	{
		key.SetStringValue(boost::nowide::widen(entry).c_str(), boost::nowide::widen(value).c_str());
	}

	key.Close();
}

void delete_value(const std::string &section, const std::string &entry, const std::string &regKey, HKEY root)
{
	std::string strKey = regKey + "\\" + section;

	ATL::CRegKey key;

	if (key.Create(root, boost::nowide::widen(strKey).c_str(), REG_NONE, REG_OPTION_NON_VOLATILE, KEY_WRITE) == ERROR_SUCCESS)
	{
		key.DeleteValue(boost::nowide::widen(entry).c_str());
	}

	key.Close();
}

void delete_key(const std::string &section, const std::string &regKey, HKEY root)
{
	ATL::CRegKey key;

	if (key.Create(root, boost::nowide::widen(regKey).c_str(), REG_NONE, REG_OPTION_NON_VOLATILE, KEY_WRITE) == ERROR_SUCCESS)
	{
		key.DeleteSubKey(boost::nowide::widen(section).c_str());
	}

	key.Close();
}
#endif // _WIN32

}

}
