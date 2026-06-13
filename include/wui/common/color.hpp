//
// Copyright (c) 2021-2026 Intent Garden Org
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
//

#pragma once

#include <cstdint>

namespace wui
{

//! Cairo. RGBA is opaque black (0.0, 0.0, 0.0, 1.0).
//! Gdiplus. ARGB is opaque black (255, 0.0, 0.0, 0.0)
//! wui *.json use BGRA format: new opaque and alpha

typedef uint32_t color; //! RGBA

//! make color RGBA, A=255
static constexpr inline color make_color(const uint8_t red, const uint8_t green, const uint8_t blue) noexcept
{
    return (0xFF000000 | red | (static_cast<uint16_t>(green) << 8)) | (static_cast<uint32_t>(blue) << 16);
}

//! make color RGBA
static constexpr inline color make_color(const uint8_t red, const uint8_t green, const uint8_t blue, const uint8_t alpha) noexcept
{
    return (red | (static_cast<uint16_t>(green) << 8)) | (static_cast<uint32_t>(blue) << 16) | (static_cast<uint32_t>(alpha) << 24);
}

//! wui *.json use BGRA format
//! make color BGRA, A=255
static constexpr inline color make_color_bgra(const uint8_t red, const uint8_t green, const uint8_t blue) noexcept
{
    return (0xFF000000 | blue | (static_cast<uint16_t>(green) << 8)) | (static_cast<uint32_t>(red) << 16);
}

//! make color BGRA
static constexpr inline color make_color_bgra(const uint8_t red, const uint8_t green, const uint8_t blue, const uint8_t alpha) noexcept
{
    return (blue | (static_cast<uint16_t>(green) << 8)) | (static_cast<uint32_t>(red) << 16) | (static_cast<uint32_t>(alpha) << 24);
}

static constexpr inline uint32_t conv_bgra_to_rgba(const color bgra) noexcept
{
    return ((bgra >> 16) & 0xFF) | (bgra & 0xFF00) | (bgra & 0xFF) | (bgra & 0xFF000000);
}

//! RGBA to RGB
static constexpr inline color get_rgb(const color rgba) noexcept
{
    return rgba & 0x00FFFFFF;
}

//! RGBA to RGB[0xFF]
static constexpr inline color get_rgb_opaqui(const color rgba) noexcept
{
    return 0xFF000000 | (rgba & 0x00FFFFFF);
}

//! RGBA is use alpha
static constexpr inline uint8_t is_alpha(const color rgba) noexcept
{
    return 255 != ((rgba >> 24) & 0xFF);
}

//! RGBA to A
static constexpr inline uint8_t get_alpha(const color rgba) noexcept
{
    return (rgba >> 24) & 0xFF;
}

//! RGBA to R
static constexpr inline uint8_t get_red(const color rgba) noexcept
{
    return rgba & 0xFF;
}

//! RGBA to G
static constexpr inline uint8_t get_green(const color rgba) noexcept
{
    return (rgba >> 8) & 0xFF;
}

//! RGBA to B
static constexpr inline uint8_t get_blue(const color rgba) noexcept
{
    return (rgba >> 16) & 0xFF;
}

//! wui *.json use BGRA format

//! BGRA to A
static constexpr inline uint8_t get_alpha_bgra(const uint32_t bgra) noexcept
{
    return (bgra >> 24) & 0xFF;
}

//! BGRA to R
static constexpr inline uint8_t get_red_bgra(const uint32_t bgra) noexcept
{
    return (bgra >> 16) & 0xFF;
}

//! BGRA to G
static constexpr inline uint8_t get_green_bgra(const uint32_t bgra) noexcept
{
    return (bgra >> 8) & 0xFF;
}

//! BGRA to B
static constexpr inline uint8_t get_blue_bgra(const uint32_t bgra) noexcept
{
    return bgra & 0xFF;
}
}
