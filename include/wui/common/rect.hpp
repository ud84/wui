//
// Copyright (c) 2021-2026 Intent Garden Org
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
//

#pragma once

#include <cstdint>
#include <algorithm>

#ifdef min
#   undef min
#endif
#ifdef max
#   undef max
#endif

namespace wui
{

struct rect
{
    int32_t left, top, right, bottom;

    inline bool operator==(const rect &lr) noexcept
    {
        return lr.left == left && lr.top == top && lr.right == right && lr.bottom == bottom;
    }

    inline bool operator>(const rect &lr) noexcept
    {
        return width() > lr.width() && height() > lr.height();
    }

    inline rect operator+(const rect &lr) noexcept
    {
        // что делает? геом. смысл
        return rect{ left + lr.left, top + lr.top, left + lr.right, top + lr.bottom };

        // может так?
        //rect r;
        //max(r, *this, lr);
        //return r;
    }

    inline void clear() noexcept
    {
        *this = rect{};
    }

    inline bool in(const int32_t x, const int32_t y) const noexcept
    {
        return x >= left && x <= right && y >= top && y <= bottom;
    }

    inline bool in(const rect &outer) const noexcept
    {
        return !((outer.right <= left || right <= outer.left || outer.bottom <= top || bottom <= outer.top));
    }

    inline bool is_null() const noexcept
    {
        return 0 == left && 0 == top && 0 == right && 0 == bottom;
    }

    inline bool empty() const noexcept
    {
        return right == left || bottom == top;
    }

    inline int32_t width() const noexcept
    {
        return right - left;
    }

    inline int32_t height() const noexcept
    {
        return bottom - top;
    }

    inline void move(const int32_t x, const int32_t y) noexcept
    {
        left += x;
        top += y;
        right += x;
        bottom += y;
    }
    inline void set(const int32_t left_, const int32_t top_) noexcept
    {
        int32_t t = right - left;
        left = left_;
        right = left_ + t;
        t = bottom - top;
        top = top_;
        bottom = top_ + t;
    }

    inline void put(const int32_t x, const int32_t y) noexcept
    {
        right = x + width();
        bottom = y + height();
        left = x;
        top = y;
    }

    inline void resize(const int32_t width_, const int32_t height_) noexcept
    {
        right = left + width_;
        bottom = top + height_;
    }

    inline void widen(const int32_t val) noexcept
    {
        left -= val;
        top -= val;
        right += val;
        bottom += val;
    }

    inline void widen(const int32_t dx, const int32_t dy) noexcept
    {
        left -= dx;
        top -= dx;
        right += dy;
        bottom += dy;
    }

    static inline rect min(const rect& r1, const rect& r2) noexcept
    {
        rect r;
        min(r, r1, r2);
        return r;
    }

    static inline void min(rect& r_out, const rect& r1, const rect& r2) noexcept
    {
        r_out.left = std::max(r1.left, r2.left);
        r_out.right = std::min(r1.right, r2.right);
        r_out.top = std::max(r1.top, r2.top);
        r_out.bottom = std::min(r1.bottom, r2.bottom);
    }

    static inline rect max(const rect &r1, const rect& r2) noexcept
    {
        rect r;
        max(r, r1, r2);
        return r;
    }
    static inline void max(rect &r_out, const rect &r1, const rect& r2) noexcept
    {
        r_out.left = std::min(r1.left, r2.left);
        r_out.right = std::max(r1.right, r2.right);
        r_out.top = std::min(r1.top, r2.top);
        r_out.bottom = std::max(r1.bottom, r2.bottom);
    }
};

}
