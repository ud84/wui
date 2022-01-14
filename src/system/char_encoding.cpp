//
// Copyright (c) 2021-2022 Anton Golovkov (udattsk at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/ud84/wui
//

#include <wui/system/char_encoding.hpp>

#ifndef _WIN32

#include <memory>
#include <cstring>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

#else

#include <windows.h>

#endif

namespace wui
{

#ifndef _WIN32

std::string to_multibyte(const std::wstring &input)
{
    size_t multiByteLen = input.size() * 2;
    std::unique_ptr<char[]> multiByteBuf(new char[multiByteLen + 1]);
    memset(multiByteBuf.get(), 0, multiByteLen + 1);

    setlocale(LC_ALL, "");
    wcstombs(multiByteBuf.get(), input.c_str(), multiByteLen);

    return std::string(multiByteBuf.get());
}

std::wstring to_widechar(const std::string &input)
{
    size_t multiByteLen = input.size();
    size_t wideCharLen = multiByteLen;
    std::unique_ptr<wchar_t[]> wideCharBuf(new wchar_t[wideCharLen + 1]);
    memset(wideCharBuf.get(), 0, (wideCharLen + 1) * sizeof(wchar_t));

    setlocale(LC_ALL, "");
    mbstowcs(wideCharBuf.get(), input.c_str(), wideCharLen);

    return std::wstring(wideCharBuf.get());
}

#else

std::string to_multibyte(const std::wstring &input)
{
	int multiByteLen = input.size();
	char *multiByteBuf = new char[multiByteLen + 1];
	memset(multiByteBuf, 0, multiByteLen+1);

	WideCharToMultiByte(CP_UTF8,
		0,
		input.c_str(),
		input.size(),
		multiByteBuf, 
		multiByteLen,
		NULL, NULL );

	std::string out = multiByteBuf;

	delete[] multiByteBuf;

	return out;
}

std::wstring to_widechar(const std::string &input)
{
	int multiByteLen = input.size();
	int wideCharLen = multiByteLen + 1;
	wchar_t *wideCharBuf = new wchar_t[wideCharLen];
	memset(wideCharBuf, 0, wideCharLen * sizeof(wchar_t));

	MultiByteToWideChar(CP_UTF8,
		0,
		input.c_str(),
		input.size(),
		wideCharBuf, 
		wideCharLen );

	std::wstring out = wideCharBuf;

	delete[] wideCharBuf;

	return out;
}

#endif

}
