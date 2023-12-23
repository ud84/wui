//
// Copyright (c) 2021-2022 Anton Golovkov (udattsk at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/ud84/wui
//

#ifdef _WIN32

#include <wui/config/config_impl_reg.hpp>
#include <wui/system/tools.hpp>

#include <boost/nowide/convert.hpp>

#include <atlbase.h>

namespace wui
{

namespace config
{

config_impl_reg::config_impl_reg(std::string_view base_application_key_, HKEY root_)
	: base_application_key(base_application_key_), root(root_)
{
}

config_impl_reg::~config_impl_reg()
{
}

int32_t config_impl_reg::get_int(std::string_view section, std::string_view entry, int32_t default_)
{
	std::string strKey = base_application_key + "\\" + std::string(section);

	ATL::CRegKey key;

	if (key.Create(root, boost::nowide::widen(strKey).c_str(), REG_NONE, REG_OPTION_NON_VOLATILE, KEY_READ) == ERROR_SUCCESS)
	{
		DWORD value;
		if (key.QueryDWORDValue(boost::nowide::widen(entry).c_str(), value) == ERROR_SUCCESS)
		{
			key.Close();
			return value;
		}
	}

	key.Close();

	return default_;
}

void config_impl_reg::set_int(std::string_view section, std::string_view entry, int32_t value)
{
	std::string strKey = base_application_key + "\\" + std::string(section);

	ATL::CRegKey key;

	if (key.Create(root, boost::nowide::widen(strKey).c_str(), REG_NONE, REG_OPTION_NON_VOLATILE, KEY_WRITE) == ERROR_SUCCESS)
	{
		key.SetDWORDValue(boost::nowide::widen(entry).c_str(), value);
	}

	key.Close();
}

int64_t config_impl_reg::get_int64(std::string_view section, std::string_view entry, int64_t default_)
{
	std::string strKey = base_application_key + "\\" + std::string(section);

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

void config_impl_reg::set_int64(std::string_view section, std::string_view entry, int64_t value)
{
	std::string strKey = base_application_key + "\\" + std::string(section);

	ATL::CRegKey key;

	if (key.Create(root, boost::nowide::widen(strKey).c_str(), REG_NONE, REG_OPTION_NON_VOLATILE, KEY_WRITE) == ERROR_SUCCESS)
	{
		key.SetQWORDValue(boost::nowide::widen(entry).c_str(), value);
	}

	key.Close();
}

std::string config_impl_reg::get_string(std::string_view section, std::string_view entry, std::string_view default_)
{
	std::string strKey = base_application_key + "\\" + std::string(section);

	std::string out(default_.begin(), default_.end());

	ATL::CRegKey key;

	if (key.Create(root, boost::nowide::widen(strKey).c_str(), REG_NONE, REG_OPTION_NON_VOLATILE, KEY_READ) == ERROR_SUCCESS)
	{
		DWORD dwcbNeeded = 0;
		if (key.QueryStringValue(boost::nowide::widen(entry).c_str(), (LPTSTR)NULL, &dwcbNeeded) == ERROR_SUCCESS)
		{
			if (dwcbNeeded > 2048)
			{
				ATLTRACE(L"Config [E] too big string in: %s, %s, %s\n", base_application_key.c_str(), section.data(), entry.data());
				return default_.data();
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

void config_impl_reg::set_string(std::string_view section, std::string_view entry, std::string_view value)
{
	std::string strKey = base_application_key + "\\" + std::string(section);

	ATL::CRegKey key;

	if (key.Create(root, boost::nowide::widen(strKey).c_str(), REG_NONE, REG_OPTION_NON_VOLATILE, KEY_WRITE) == ERROR_SUCCESS)
	{
		key.SetStringValue(boost::nowide::widen(entry).c_str(), boost::nowide::widen(value).c_str());
	}

	key.Close();
}

void config_impl_reg::delete_value(std::string_view section, std::string_view entry)
{
	std::string strKey = base_application_key + "\\" + std::string(section);

	ATL::CRegKey key;

	if (key.Create(root, boost::nowide::widen(strKey).c_str(), REG_NONE, REG_OPTION_NON_VOLATILE, KEY_WRITE) == ERROR_SUCCESS)
	{
		key.DeleteValue(boost::nowide::widen(entry).c_str());
	}

	key.Close();
}

void config_impl_reg::delete_key(std::string_view section)
{
	ATL::CRegKey key;

	if (key.Create(root, boost::nowide::widen(base_application_key).c_str(), REG_NONE, REG_OPTION_NON_VOLATILE, KEY_WRITE) == ERROR_SUCCESS)
	{
		key.DeleteSubKey(boost::nowide::widen(section).c_str());
	}

	key.Close();
}

error config_impl_reg::get_error() const
{
    return {};
}

}

}

#endif
