//
// Copyright (c) 2021-2022 Anton Golovkov (udattsk at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/ud84/wui
//

#include <wui/common/char_helpers.hpp>

#include <locale>
#include <codecvt>

namespace wui
{

std::string to_multibyte(const std::wstring &input)
{
	const std::u16string& utf16 = reinterpret_cast<const char16_t*>(input.c_str());
	
	std::wstring_convert<std::codecvt_utf8_utf16<int16_t>, int16_t> convert;
	auto p = reinterpret_cast<const int16_t*>(utf16.data());
	std::string utf8_string;

	try
	{
		utf8_string = convert.to_bytes(p, p + utf16.size());
	}
	catch (std::range_error &)
	{
		printf("bad conversion in toMultiByte\n");
	}

	return utf8_string;
}

std::wstring to_widechar(const std::string &input)
{
	std::wstring_convert<std::codecvt_utf8_utf16<int16_t>, int16_t> convert;
	std::u16string utf16_string;

	try
	{
		utf16_string = reinterpret_cast<const char16_t*>(convert.from_bytes(input).data());
	}
	catch (std::range_error &)
	{
		printf("bad conversion in utf8ToUtf16\n");
	}

	return reinterpret_cast<const wchar_t*>(utf16_string.c_str());
}

}
