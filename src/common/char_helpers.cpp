//
// Copyright (c) 2021-2022 Anton Golovkov (udattsk at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/ud84/wui
//

#include <wui/common/char_helpers.hpp>

#ifndef _WIN32

#include <memory>
#include <cstring>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

#else

#include <codecvt>

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
        printf("bad conversion in wui::to_multibyte\n");
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
        printf("bad conversion in wui::to_widechar\n");
    }

    return reinterpret_cast<const wchar_t*>(utf16_string.c_str());
}

#endif

}
