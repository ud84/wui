// Copyright (c) 2026 Intent Garden Org. Boost Software License 1.0.
#pragma once
#include <wui/common/error.hpp>
#include <string_view>
#include <vector>
#include <cstdint>
struct wasm_image { int id, width, height; };
void load_image_from_data(const std::vector<uint8_t>& data, void **image);
void load_image_from_file(std::string_view file, std::string_view path, void **image, wui::error& error);
void free_image(void **image);
int wasm_image_width(void *image);
int wasm_image_height(void *image);
